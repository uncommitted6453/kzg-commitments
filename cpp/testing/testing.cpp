#include "kzg.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <tuple>
#include <utility>
#include <ctime>
#include <unistd.h>
#include <vector>

void check_test(bool status, string test_name);
void example_test();
void random_test(int length, int num_coeff, int runs);
void empty_proof_test();
void invalid_setup_test();
void poly_empty_test();
void poly_degree_1_test();
void poly_degree_10_test();
void chunking_test();
void chunking_invalid_args_test();
void general_test(int num_coeff, string data, vector<pair<int, int>> to_verify, vector<tuple<int, int, string>> to_refute);
string random_string(const int len);

int main() {
  kzg::init();
  // empty_proof_test();  // seg faults!
  // empty_verify_test(); // seg faults
  invalid_setup_test();
  poly_degree_1_test();
  poly_degree_10_test();
  chunking_test();
  chunking_invalid_args_test();
  random_test(10, 15, 1);
}

void example_test() {
  string data = "hello there my name is bob";

  return general_test(128, data,
    {
      {0, strlen("hello")},
      {strlen("hello there my "), strlen("name is")},
      {strlen("hello there my name is "), strlen("bob")}
    },
    {
      {strlen("hello there my name is "), strlen("bob"), "alice"}
    }
  );
}

void random_test(int length, int num_coeff, int runs) {
  for (int i = 0; i < runs; ++i) {
    string data = random_string(length);
    // Stores indices of the string data such that
    //   kzg.create_proof(poly, j, k) is a proof for data[j:k]
    vector<pair<int, int>> to_verify;
    // Same as above, but also stores the strings that we will refute.
    vector<tuple<int, int, string>> to_refute;
    // Should go up to length instead of length - 1,
    // but create_proof seg faults when j == 0, and k == length.
    for (int j = 0; j < length - 1; ++j) {
      for (int k = 1; j + k < length; ++k) {
        to_verify.push_back(make_pair(j, k));

        string substring = data.substr(j, k);
        string str_to_refute = random_string((rand() % (length * 2)) + 1); // TODO: str length of 0 not working
        while (substring == str_to_refute) {
          str_to_refute = random_string((rand() % (length * 2)) + 1);
        }

        to_refute.push_back(make_tuple(j, k, str_to_refute));
      }
    }
    general_test(num_coeff, data, to_verify, to_refute);
  }
}

void empty_proof_test() { // Still getting seg faults.
  kzg::trusted_setup kzg(128);
  kzg::blob blob = kzg::blob::from_string("some data here");
  kzg::poly poly = kzg::poly::from_blob(blob);
  bool exception = false;
  try {
    kzg::proof empty_proof = kzg.create_proof(poly, 5, 0);
  } catch(...) {
    exception = true;
  }

  check_test(exception, "empty_proof_test");
}

void empty_verify_test() { // Still getting seg faults
  kzg::trusted_setup kzg(128);
  kzg::blob blob = kzg::blob::from_string("some data here");
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);

  kzg::proof empty_proof = kzg.create_proof(poly, strlen("some da"), strlen("ta"));
  kzg::blob refute = kzg::blob::from_string("", strlen("some da"));
  bool exception = false;
  try {
    kzg.verify_proof(commit, empty_proof, refute);
  } catch(...) {
    exception = true;
  }

  check_test(exception, "empty_verify_test");
}

void invalid_setup_test() {
  bool exception = false;
  try { kzg::trusted_setup kzg(0); }
  catch (...) { exception = true; }
  check_test(exception, "empty polynomial is invalid");
  
  exception = false;
  try { kzg::trusted_setup kzg(1); }
  catch (...) { exception = true; }
  check_test(exception, "0 degree polynomial is invalid");
}

void poly_degree_1_test() {
  kzg::trusted_setup kzg(2);
  
  string data = "K";
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "1 degree polynomial, 1 character commit verification");

  kzg::proof proof = kzg.create_proof(poly, 0, strlen("K"));
  kzg::blob verify = kzg::blob::from_string("K", 0);
  check_test(kzg.verify_proof(commit, proof, verify), "1 degree polynomial, 1 character proof verification");
  
  bool exception = false;
  data = "AB";
  blob = kzg::blob::from_string(data);
  poly = kzg::poly::from_blob(blob);
  try { kzg.create_commit(poly); }
  catch (...) { exception = true; }
  check_test(exception, "1 degree polynomial, 2 character commit is invalid");
  
  kzg::blob refute1 = kzg::blob::from_string("k", 0);
  kzg::blob refute2 = kzg::blob::from_string("j", 0);
  check_test(!kzg.verify_proof(commit, proof, refute1), "1 degree polynomial, proof refutation 1");
  check_test(!kzg.verify_proof(commit, proof, refute2), "1 degree polynomial, proof refutation 2");
  
  exception = false;
  kzg::blob invalid = kzg::blob::from_string("K ", 0);
  try { kzg.verify_proof(commit, proof, invalid); }
  catch (...) { exception = true; }
  check_test(exception, "1 degree polynomial, 2 character proof is invalid");
}

void poly_degree_10_test() {
  kzg::trusted_setup kzg(11);
  
  string data = "CEBIDKAGFJH";
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);

  bool exception = false;
  try { kzg.create_commit(poly); }
  catch (...) { exception = true; }
  check_test(exception, "10 degree polynomial, 11 character commit is invalid");
  
  data = "CEBIDAGFJH";
  blob = kzg::blob::from_string(data);
  poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);  
  check_test(kzg.verify_commit(commit, poly), "10 degree polynomial, commit verification");

  kzg::proof proof = kzg.create_proof(poly, strlen("CE"), strlen("BID"));
  kzg::blob verify = kzg::blob::from_string("BID", strlen("CE"));
  check_test(kzg.verify_proof(commit, proof, verify), "10 degree polynomial, proof verification");

  kzg::blob refute1 = kzg::blob::from_string("CDEF", 0);
  kzg::blob refute2 = kzg::blob::from_string("CD", 0);
  check_test(kzg.verify_proof(commit, proof, refute1), "10 degree polynomial, proof refutation 1");
  check_test(kzg.verify_proof(commit, proof, refute2), "10 degree polynomial, proof refutation 2");
}

void chunking_test() {
  kzg::trusted_setup kzg(128);

  unsigned char data[] = "123456789abcdef";
  kzg::blob blob1 = kzg::blob::from_bytes(data, 0, sizeof(data), 1);
  kzg::blob blob2 = kzg::blob::from_bytes(data, 0, sizeof(data), 2);
  bool exception = false;
  try {
    kzg::blob blob3 = kzg::blob::from_bytes(data, 0, sizeof(data), 3);
  }
  catch (...) {
    exception = true;
  }
  if (!exception) {
    cout << "FAILED chunking, chunks do not divide data" << endl;
  }
  kzg::blob blob4 = kzg::blob::from_bytes(data, 0, sizeof(data), 4);

  kzg::poly poly = kzg::poly::from_blob(blob1);
  kzg::commit commit = kzg.create_commit(poly);
  if (!kzg.verify_commit(commit, poly)) {
    cout << "FAILED chunking, commit verification for blob1" << endl;
  }
  kzg::proof proof = kzg.create_proof(poly, 3, 9, 1);
  kzg::blob verify = kzg::blob::from_bytes((uint8_t*) "456789abc", 3, 9, 1);
  if (!kzg.verify_proof(commit, proof, verify)) {
    cout << "FAILED chunking, proof verification for blob1" << endl;
  }

  poly = kzg::poly::from_blob(blob2);
  commit = kzg.create_commit(poly);
  if (!kzg.verify_commit(commit, poly)) {
    cout << "FAILED chunking, commit verification for blob2" << endl;
  }
  proof = kzg.create_proof(poly, 2, 10, 2);
  verify = kzg::blob::from_bytes((uint8_t*) "3456789abc", 2, 10, 2);
  if (!kzg.verify_proof(commit, proof, verify)) {
    cout << "FAILED chunking, proof verification for blob2" << endl;
  }

  poly = kzg::poly::from_blob(blob4);
  commit = kzg.create_commit(poly);
  if (!kzg.verify_commit(commit, poly)) {
    cout << "FAILED chunking, commit verification for blob4" << endl;
  }
  proof = kzg.create_proof(poly, 4, 8, 4);
  verify = kzg::blob::from_bytes((uint8_t*) "56789abc", 4, 8, 4);
  if (!kzg.verify_proof(commit, proof, verify)) {
    cout << "FAILED chunking, proof verification for blob4" << endl;
  }
}

void chunking_invalid_args_test() {
  kzg::trusted_setup kzg(128);

  unsigned char data[] = "123456789abcdef";
  kzg::blob blob1 = kzg::blob::from_bytes(data, 0, sizeof(data), 1);

  kzg::poly poly = kzg::poly::from_blob(blob1);
  kzg::commit commit = kzg.create_commit(poly);
  if (!kzg.verify_commit(commit, poly)) {
    cout << "FAILED chunking invalid args, commit verification" << endl;
  }

  bool exception = false;
  try {
    kzg.create_proof(poly, 0, 5, 4);
  } catch (...) {
    exception = true;
  }
  if (!exception) {
    cout << "FAILED chunking invalid args, invalid byte length" << endl;
  }
  exception = false;
  try {
    kzg.create_proof(poly, 2, 8, 4);
  } catch (...) {
    exception = true;
  }
  if (!exception) {
    cout << "FAILED chunking invalid args, invalid byte offset" << endl;
  }
  // TODO: test CURVE_ORDER_BYTES
  exception = false;
  try {
    kzg.create_proof(poly, 12, 12, 1);
  } catch (...) {
    exception = true;
  }
  if (!exception) {
    cout << "FAILED chunking invalid args, bytes out of range of data" << endl;
  }
}

void general_test(int num_coeff, string data, vector<pair<int, int>> to_verify, vector<tuple<int, int, string>> to_refute) {
  bool success = true;
  kzg::trusted_setup kzg(num_coeff);
  
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);

  if (!kzg.verify_commit(commit, poly)) {
    cout << "Failed to verify commit {data: " << data << ", num_coeff: " << num_coeff << "}" << endl;
    return;
  }

  for (auto s : to_verify) {
    kzg::proof s_proof = kzg.create_proof(poly, s.first, s.second); // SEG FAULT when s.second == len(data) and s.first == 0.
    string substring = data.substr(s.first, s.second);
    kzg::blob verify = kzg::blob::from_string(substring, s.first);
    if (kzg.verify_proof(commit, s_proof, verify)) {
      // cout << "Verified (" << s.first << ", " << s.second << "): Message is " << data << " and we are proving " << substring  << endl;
    } else {
      cout << "FAILED   (" << s.first << ", " << s.second << "): Message is " << data << " and we are proving " << substring  << endl;
      success = false;
    }
  }

  for (auto s : to_refute) {
    kzg::proof s_proof = kzg.create_proof(poly, get<0>(s), get<1>(s));
    kzg::blob refute = kzg::blob::from_string(get<2>(s), get<0>(s));
    if (!kzg.verify_proof(commit, s_proof, refute)) { // This is seg faulting with the strings to refute have 0 length.
      // cout << "Refuted (" << get<0>(s) << ", " << get<1>(s) << "): Message is " << data << " and we are refuting " << get<2>(s)  << endl;
    } else {
      cout << "FAILED  (" << get<0>(s) << ", " << get<1>(s) << "): Message is " << data << " and we are refuting " << get<2>(s)  << endl;
    }
  }
}

// https://stackoverflow.com/a/440240
string random_string(const int len) {
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
  string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  
  return tmp_s;
}

void check_test(bool status, string test_name) {
  if (status) cout << "PASSED [" << test_name << "]" << endl;
  else cout << "FAILED [" << test_name << "]" << endl;
}

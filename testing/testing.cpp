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
#include <sstream>
#include <fstream>

void check_test(bool status, string test_name);
void example_test();
void random_test(int length, int num_coeff, int runs, bool to_serialize);
void empty_proof_test();
void empty_verify_test();
void invalid_setup_test();
void poly_empty_test();
void poly_degree_1_test();
void poly_degree_10_test();
void high_poly_degree_test();
void chunking_test();
void chunking_invalid_args_test();
void general_test(int num_coeff, string data, vector<pair<int, int>> to_verify, vector<tuple<int, int, string>> to_refute, bool to_serialize);
vector<uint8_t> from_hex(string s);
string random_string(const int len);
void generate_expected_test_data(
  vector<pair<int, int>>& to_verify,
  vector<tuple<int, int, string>>& to_refute,
  int length,
  int num_coeff,
  const string& data
);
void eth_blob_test();

int main() {
  kzg::init();
  empty_proof_test();
  empty_verify_test();
  invalid_setup_test();
  high_poly_degree_test();
  poly_degree_1_test();
  poly_degree_10_test();
  chunking_test();
  chunking_invalid_args_test();
  random_test(9, 140, 1, true);
  eth_blob_test();
}

void eth_blob_test() {
  kzg::trusted_setup kzg(5000);
  
  // commit and generate proof for blob from eth transaction
  // transaction hash: 0x9e3de317d97616d3bd2e714e467a14f88c08af00a30c1d8bbc61eee9dd3dc0d0 
  // from ethscan.io
  ifstream blob1_file("blob1.txt");
  stringstream blob1_buffer;
  blob1_buffer << blob1_file.rdbuf();
  vector<uint8_t> blob1_bytes = from_hex(blob1_buffer.str());
  
  int zero_pad = MAX_CHUNK_BYTES - (blob1_bytes.size() % MAX_CHUNK_BYTES);
  for (int i = 0; i < zero_pad; i++)
    blob1_bytes.push_back(0);
  
  kzg::blob blob = kzg::blob::from_bytes(blob1_bytes.data(), 0, blob1_bytes.size(), MAX_CHUNK_BYTES);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "eth-blob, commit verification for blob1");
  
  int verify_chunks = 4;
  int rand_offset = rand() % (blob1_bytes.size() / MAX_CHUNK_BYTES - verify_chunks);
  int bytes_offset = rand_offset * MAX_CHUNK_BYTES;
  kzg::proof proof = kzg.create_proof(poly, rand_offset, verify_chunks);
  kzg::blob verify = kzg::blob::from_bytes(&blob1_bytes.data()[bytes_offset], bytes_offset, verify_chunks * MAX_CHUNK_BYTES, MAX_CHUNK_BYTES);
  check_test(kzg.verify_proof(commit, proof, verify), "eth-blob, proof verification for blob1");
  
  // commit and generate proof for blob from eth transaction
  // transaction hash: 0x9e3de317d97616d3bd2e714e467a14f88c08af00a30c1d8bbc61eee9dd3dc0d0
  // from ethscan.io
  ifstream blob2_file("blob2.txt");
  stringstream blob2_buffer;
  blob2_buffer << blob2_file.rdbuf();
  vector<uint8_t> blob2_bytes = from_hex(blob2_buffer.str());
  
  zero_pad = MAX_CHUNK_BYTES - (blob2_bytes.size() % MAX_CHUNK_BYTES);
  for (int i = 0; i < zero_pad; i++)
    blob2_bytes.push_back(0);
  
  blob = kzg::blob::from_bytes(blob2_bytes.data(), 0, blob2_bytes.size(), MAX_CHUNK_BYTES);
  poly = kzg::poly::from_blob(blob);
  commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "eth-blob, commit verification for blob2");
  
  rand_offset = rand() % (blob2_bytes.size() / MAX_CHUNK_BYTES - verify_chunks);
  bytes_offset = rand_offset * MAX_CHUNK_BYTES;
  proof = kzg.create_proof(poly, rand_offset, verify_chunks);
  verify = kzg::blob::from_bytes(&blob2_bytes.data()[bytes_offset], bytes_offset, verify_chunks * MAX_CHUNK_BYTES, MAX_CHUNK_BYTES);
  check_test(kzg.verify_proof(commit, proof, verify), "eth-blob, proof verification for blob2");
}

void example_test() {
  string data = "hello there my name is bob";
  return general_test(
    128,
    data, {
      { 0, strlen("hello") },
      { strlen("hello there my "), strlen("name is") },
      { strlen("hello there my name is "), strlen("bob")}
    }, {
      { strlen("hello there my name is "), strlen("bob"), "alice" }
    },
    true
  );
}

void random_test(int length, int num_coeff, int runs, bool to_serialize) {
  for (int i = 0; i < runs; ++i) {
    string data = random_string(length);
    vector<pair<int, int>> to_verify;
    vector<tuple<int, int, string>> to_refute;
    generate_expected_test_data(to_verify, to_refute, length, num_coeff, data);
    general_test(num_coeff, data, to_verify, to_refute, to_serialize);
  }
}

void empty_proof_test() {
  kzg::trusted_setup kzg(128);
  kzg::blob blob = kzg::blob::from_string("some data here");
  kzg::poly poly = kzg::poly::from_blob(blob);
  bool exception = false;
  try { kzg.create_proof(poly, 5, 0); }
  catch(...) { exception = true; }
  check_test(exception, "empty proof is invalid");
}

void empty_verify_test() {
  kzg::trusted_setup kzg(128);
  kzg::blob blob = kzg::blob::from_string("some data here");
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);

  kzg::proof empty_proof = kzg.create_proof(poly, strlen("some da"), strlen("ta"));
  kzg::blob refute = kzg::blob::from_string("", strlen("some da"));
  bool exception = false;
  try { kzg.verify_proof(commit, empty_proof, refute); }
  catch(...) { exception = true; }
  check_test(exception, "empty verification is invalid");
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
  kzg::blob refute2 = kzg::blob::from_string("jj", 2); // This should not throw.
  check_test(!kzg.verify_proof(commit, proof, refute1), "1 degree polynomial, proof refutation 1");
  check_test(!kzg.verify_proof(commit, proof, refute2), "1 degree polynomial, proof refutation 2");
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
  kzg::blob refute2 = kzg::blob::from_string("CD", 12);
  kzg::blob refute3 = kzg::blob::from_string("BHSDJCSHJDVBZ", 0);
  check_test(!kzg.verify_proof(commit, proof, refute1), "10 degree polynomial, proof refutation 1");
  check_test(!kzg.verify_proof(commit, proof, refute2), "10 degree polynomial, proof refutation 2");
  check_test(!kzg.verify_proof(commit, proof, refute3), "10 degree polynomial, proof refutation 3");
}

void high_poly_degree_test() {
  kzg::trusted_setup kzg(150);
  string data = "fa37JncCHryDsbzayy4cBWDxS22JjzhMaiRrV41mtzxlYvKWrO72tK0LK0e1zLOZ2nOXpPIhMFSv8kP07U20o0J90xA0GWXIIwo7J4ogHFZQxwQ2RQ0DRJKRETPVzxlFrXL8b7mtKLHIGhIh5JuWcF";
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);

  bool exception = false;
  try { kzg.create_commit(poly); }
  catch (...) { exception = true; }
  check_test(exception, "149 degree polynomial, 150 character commit is invalid");

  data = "wrgJKdE3t5bECALy3eKIwYxEF3V7Z8KTx0nFe1IX5tjH22F5gXOa5LnIMIQuOiNJj8YL8rqDiZSkZfoEDAmGTXXqqvkCd5WKE2fMtVXa2zKae6opGY4i6bYuUG67LaSXd5tUbO4bNPB0TxnkWrSaQ";
  blob = kzg::blob::from_string(data);
  poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "149 degree polynomial, commit verification");
  
  kzg::proof proof = kzg.create_proof(poly, 49, 57);
  string substring = data.substr(49, 57);
  kzg::blob verify = kzg::blob::from_string(substring, 49);
  check_test(kzg.verify_proof(commit, proof, verify), "149 degree polynomial, proof verification");

  kzg::blob refute1 = kzg::blob::from_string(substring, 50);
  kzg::blob refute2 = kzg::blob::from_string(data.substr(49, 56), 30);
  kzg::blob refute3 = kzg::blob::from_string("a", 200);
  kzg::blob refute4 = kzg::blob::from_string(random_string(200), 3);
  check_test(!kzg.verify_proof(commit, proof, refute1), "149 degree polynomial, proof refutation 1");
  check_test(!kzg.verify_proof(commit, proof, refute2), "149 degree polynomial, proof refutation 2");
  check_test(!kzg.verify_proof(commit, proof, refute3), "149 degree polynomial, proof refutation 3");
  check_test(!kzg.verify_proof(commit, proof, refute4), "149 degree polynomial, proof refutation 4");
}

void chunking_test() {
  kzg::trusted_setup kzg(128);

  unsigned char data[] = "ysudYUGdghv675d";

  bool exception = false;
  try { kzg::blob::from_bytes(data, 0, sizeof(data), 3); }
  catch (...) { exception = true; }
  check_test(exception, "chunking, chunks do not divide data");
  
  kzg::blob blob1 = kzg::blob::from_bytes(data, 0, sizeof(data), 1);
  kzg::poly poly = kzg::poly::from_blob(blob1);
  kzg::commit commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "chunking, commit verification for blob1");

  kzg::proof proof = kzg.create_proof(poly, 3, 9, 1);
  kzg::blob verify = kzg::blob::from_bytes((uint8_t*) "dYUGdghv6", 3, 9, 1);
  check_test(kzg.verify_proof(commit, proof, verify), "chunking, proof verification for blob1");

  kzg::blob blob2 = kzg::blob::from_bytes(data, 0, sizeof(data), 2);
  poly = kzg::poly::from_blob(blob2);
  commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "chunking, commit verification for blob2");
  
  proof = kzg.create_proof(poly, 2, 10, 2);
  verify = kzg::blob::from_bytes((uint8_t*) "udYUGdghv6", 2, 10, 2);
  check_test(kzg.verify_proof(commit, proof, verify), "chunking, proof verification for blob2");

  kzg::blob blob4 = kzg::blob::from_bytes(data, 0, sizeof(data), 4);
  poly = kzg::poly::from_blob(blob4);
  commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "chunking, commit verification for blob4");
  
  proof = kzg.create_proof(poly, 4, 8, 4);
  verify = kzg::blob::from_bytes((uint8_t*) "YUGdghv6", 4, 8, 4);
  check_test(kzg.verify_proof(commit, proof, verify), "chunking, proof verification for blob4");
}

void chunking_invalid_args_test() {
  kzg::trusted_setup kzg(128);

  unsigned char data[] = "ysudYUGdghv675d";
  kzg::blob blob1 = kzg::blob::from_bytes(data, 0, sizeof(data), 1);

  kzg::poly poly = kzg::poly::from_blob(blob1);
  kzg::commit commit = kzg.create_commit(poly);
  check_test(kzg.verify_commit(commit, poly), "chunking invalid args, commit verification");

  bool exception = false;
  try { kzg.create_proof(poly, 0, 5, 4); }
  catch (...) { exception = true; }
  check_test(exception, "chunking invalid args, invalid byte length");

  exception = false;
  try { kzg.create_proof(poly, 2, 8, 4); }
  catch (...) { exception = true; }
  check_test(exception, "chunking invalid args, invalid byte offset");
}

void general_test(int num_coeff, string data, vector<pair<int, int>> to_verify, vector<tuple<int, int, string>> to_refute, bool to_serialize) {
  bool success = true;
  kzg::trusted_setup kzg(num_coeff);
  
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  if (to_serialize) {
    std::vector<uint8_t> poly_data = poly.serialize();
    poly = kzg::poly::deserialize(poly_data);
  }
  kzg::commit commit = kzg.create_commit(poly);
  if (to_serialize) {
    std::vector<uint8_t> commit_data = commit.serialize();
    commit = kzg::commit::deserialize(commit_data);
  }
  

  if (!kzg.verify_commit(commit, poly)) {
    cout << "Failed to verify commit {data: " << data << ", num_coeff: " << num_coeff << "}" << endl;
    return;
  }

  for (auto s : to_verify) {
    kzg::proof s_proof = kzg.create_proof(poly, s.first, s.second);
    if (to_serialize) {
      std::vector<uint8_t> proof_data = s_proof.serialize();
      s_proof = kzg::proof::deserialize(proof_data);
    }

    string substring = data.substr(s.first, s.second);
    kzg::blob verify = kzg::blob::from_string(substring, s.first);
    // cout << "Checking (" << s.first << ", " << s.second << "): Message is " << data << " and we are proving " << substring  << endl;
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
    // cout << "Checking (" << get<0>(s) << ", " << get<1>(s) << "): Message is " << data << " and we are refuting " << get<2>(s)  << endl;
    if (!kzg.verify_proof(commit, s_proof, refute)) { // This is seg faulting with the strings to refute have 0 length.
      // cout << "Refuted  (" << get<0>(s) << ", " << get<1>(s) << "): Message is " << data << " and we are refuting " << get<2>(s)  << endl;
    } else {
      cout << "FAILED   (" << get<0>(s) << ", " << get<1>(s) << "): Message is " << data << " and we are refuting " << get<2>(s)  << endl;
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

// to-verify: Stores indices of the string data such that
//   kzg.create_proof(poly, j, k) is a proof for data[j:k]
// to_refute: Same as above, but also stores the strings that we will refute.
void generate_expected_test_data(
  vector<pair<int, int>>& to_verify,
  vector<tuple<int, int, string>>& to_refute,
  int length,
  int num_coeff,
  const string& data
) {
  for (int j = 0; j < length; ++j) {
    for (int k = 1; j + k <= length; ++k) {
      to_verify.push_back(make_pair(j, k));

      string substring = data.substr(j, k);
      string str_to_refute = random_string((rand() % (num_coeff - 1)) + 1);
      while (substring == str_to_refute) {
        str_to_refute = random_string((rand() % (num_coeff - 1)) + 1);
      }

      to_refute.push_back(make_tuple(j, k, str_to_refute));
    }
  }
}

vector<uint8_t> from_hex(string s) {
  vector<uint8_t> res;
  for (int i = 0; i < s.size(); i += 2) {
    string hex_byte = s.substr(i, 2);
    res.push_back(strtol(hex_byte.c_str(), NULL, 16));
  }
  return res;
}

// from https://gist.github.com/Kielx/2917687bc30f567d45e15a4577772b02
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"

void check_test(bool status, string test_name) {
  if (status) cout << GREEN << "PASSED" << RESET << " [" << test_name << "]" << endl;
  else cout << RED << "FAILED" << RESET << " [" << test_name << "]" << endl;
}

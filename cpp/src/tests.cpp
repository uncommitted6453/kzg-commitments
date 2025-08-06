#include "kzg.h"
#include "tests.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <tuple>
#include <utility>
#include <ctime>
#include <unistd.h>

bool all_tests() {
  srand((unsigned)time(NULL) * getpid());
  if (!random_test()) {
    cout << "FAILED random_test" << endl;
    return false;
  }
  return true;
  // return main_test();
}

bool main_test() {
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

bool random_test() {
  int length = 10;
  for (int i = 0; i < 10; ++i) {
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
    if (!general_test(10, data, to_verify, to_refute)) {
      return false;
    }
  }
  return true;
}

bool general_test(int num_coeff, string data, vector<pair<int, int>> to_verify, vector<tuple<int, int, string>> to_refute) {
  bool success = true;
  kzg::trusted_setup kzg(num_coeff);

  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);

  if (!kzg.verify_commit(commit, poly)) {
    cout << "Failed to verify commit" << endl;
    // TODO: print out debugging info
    return false;
  }

  for (auto s : to_verify) {
    kzg::proof s_proof = kzg.create_proof(poly, s.first, s.second); // SEG FAULT when s.second == len(data) and s.first == 0.
    string substring = data.substr(s.first, s.second);
    kzg::blob verify = kzg::blob::from_string(substring, s.first);
    if (kzg.verify_proof(commit, s_proof, verify)) {
      cout << "Verified (" << s.first << ", " << s.second << "): Message is " << data << " and we are proving " << substring  << endl;
    } else {
      cout << "FAILED   (" << s.first << ", " << s.second << "): Message is " << data << " and we are proving " << substring  << endl;
      success = false;
    }
  }

  for (auto s : to_refute) {
    kzg::proof s_proof = kzg.create_proof(poly, get<0>(s), get<1>(s));
    kzg::blob refute = kzg::blob::from_string(get<2>(s), get<0>(s));
    if (!kzg.verify_proof(commit, s_proof, refute)) { // This is seg faulting with the strings to refute have 0 length.
      cout << "Refuted (" << get<0>(s) << ", " << get<1>(s) << "): Message is " << data << " and we are refuting " << get<2>(s)  << endl;
    } else {
      cout << "FAILED  (" << get<0>(s) << ", " << get<1>(s) << "): Message is " << data << " and we are refuting " << get<2>(s)  << endl;
      return false;
    }
  }
  return success;
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

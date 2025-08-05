#include "kzg.h"
#include "tests.h"
#include <cstring>
#include <iostream>
#include <tuple>
#include <utility>

bool all_tests() {
  return main_test();
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

bool general_test(int num_coeff, string data, vector<pair<int, int>> to_verify, vector<tuple<int, int, string>> to_refute) {
  bool success = true;
  cout << "RUNNING TEST\n";
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
    kzg::proof s_proof = kzg.create_proof(poly, s.first, s.second);
    string substring = data.substr(s.first, s.second);
    kzg::blob verify = kzg::blob::from_string(substring, s.first);
    if (kzg.verify_proof(commit, s_proof, verify)) {
      cout << "Verified: " << substring << endl;
    } else {
      success = false;
      cout << "FAILED to verify: " << substring << endl;
    }
  }

  for (auto s : to_refute) {
    kzg::proof s_proof = kzg.create_proof(poly, get<0>(s), get<1>(s));
    string substring = data.substr(get<0>(s), get<1>(s));
    kzg::blob refute = kzg::blob::from_string(get<2>(s), get<0>(s));
    if (!kzg.verify_proof(commit, s_proof, refute)) {
      cout << "Refuted: " << get<2>(s) << endl;
    } else {
      success = false;
      cout << "FAILED to refute: " << get<2>(s) << endl;
    }
  }

  return success;
}

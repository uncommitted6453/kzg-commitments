#include "kzg.h"
#include "tests.h"
#include <cstring>
#include <iostream>
#include <utility>

void all_tests() {
  main_test();
}

void main_test() {
  string data = "hello there my name is bob";

  general_test(128, data, {
    {0, strlen("hello")},
    {strlen("hello there my "), strlen("name is")},
    {strlen("hello there my name is "), strlen("bob")}
  });
}

void general_test(int num_coeff, string data, vector<pair<int, int>> to_verify) {
  cout << "RUNNING TEST\n";
  kzg::trusted_setup kzg(num_coeff);

  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);

  if (!kzg.verify_commit(commit, poly)) {
    cout << "Failed to verify commit" << endl;
    // TODO: print out debugging info
    return;
  }

  for (auto s : to_verify) {
    kzg::proof s_proof = kzg.create_proof(poly, s.first, s.second);
    string substring = data.substr(s.first, s.second);
    kzg::blob verify = kzg::blob::from_string(substring, s.first);
    if (kzg.verify_proof(commit, s_proof, verify)) {
      cout << "Verified: " << substring << endl;
    } else {
      cout << "Failed to verify: " << substring << endl;
    }
  }
}

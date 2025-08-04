#include <iostream>
#include <vector>
#include "kzg.h"
#include "util.h"

using namespace std;
using namespace NTL;

int main(int argc, char *argv[]) {
  /* ----- setup ------ */
  string data = "hello there my name is bob";
  kzg::trusted_setup kzg(data.size());
  
  /* ----- commit ------ */
  vector<pair<ZZ_p, ZZ_p>> points;
  create_points_from_string(points, data, 0);
  ZZ_pX P = polyfit(points);
  kzg::commit commit = kzg.create_commit(P);
  
  /* ----- create proof------ */
  ECP hello_proof = kzg.create_proof(P, 0, strlen("hello"));
  ECP name_proof = kzg.create_proof(P, strlen("hello there my "), strlen("name is"));
  ECP bob_proof = kzg.create_proof(P, strlen("hello there my name is "), strlen("bob"));
  
  /* ------ verify ------- */
  vector<pair<ZZ_p, ZZ_p>> verify;
  
  create_points_from_string(verify, "hello", 0);
  if (kzg.verify_proof(commit, hello_proof, verify)) cout << "verified: hello" << endl;
  verify.clear();
  
  create_points_from_string(verify, "name is", strlen("hello there my "));
  if (kzg.verify_proof(commit, name_proof, verify)) cout << "verified: name is" << endl;
  verify.clear();
  
  create_points_from_string(verify, "bob", strlen("hello there my name is "));
  if (kzg.verify_proof(commit, bob_proof, verify)) cout << "verified: bob" << endl;
  verify.clear();
  
  create_points_from_string(verify, "alice", strlen("hello there my name is "));
  if (!kzg.verify_proof(commit, bob_proof, verify)) cout << "verified: not alice" << endl;
  verify.clear();
  
  return 0;
}

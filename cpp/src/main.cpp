#include <iostream>
#include <vector>
#include "kzg.h"
#include "util.h"

using namespace std;
using namespace NTL;

int main(int argc, char *argv[]) {
  /* ----- setup ------ */
  string data = "hello there my name is bob";
  kzg::public_params setup1 = kzg::trusted_setup(data.size());
  kzg::export_params_file(setup1);
  
  kzg::public_params setup = kzg::load_params_file("kzg_public");
  
  /* ----- commit ------ */
  vector<pair<ZZ_p, ZZ_p>> points;
  create_points_from_string(points, data, 0);
  ZZ_pX P = polyfit(points);
  ECP commit_result = kzg::commit(setup, P);
  
  /* ----- create proof------ */
  ECP hello_proof = kzg::create_proof(setup, P, 0, strlen("hello"));
  ECP name_proof = kzg::create_proof(setup, P, strlen("hello there my "), strlen("name is"));
  ECP bob_proof = kzg::create_proof(setup, P, strlen("hello there my name is "), strlen("bob"));
  
  /* ------ verify ------- */
  vector<pair<ZZ_p, ZZ_p>> verify;
  
  create_points_from_string(verify, "hello", 0);
  if (kzg::verify(setup, commit_result, hello_proof, verify)) cout << "verified: hello" << endl;
  verify.clear();
  
  create_points_from_string(verify, "name is", strlen("hello there my "));
  if (kzg::verify(setup, commit_result, name_proof, verify)) cout << "verified: name is" << endl;
  verify.clear();
  
  create_points_from_string(verify, "bob", strlen("hello there my name is "));
  if (kzg::verify(setup, commit_result, bob_proof, verify)) cout << "verified: bob" << endl;
  verify.clear();
  
  create_points_from_string(verify, "alice", strlen("hello there my name is "));
  if (!kzg::verify(setup, commit_result, bob_proof, verify)) cout << "verified: not alice" << endl;
  verify.clear();
  
  return 0;
}

#include <iostream>
#include <vector>
#include <cstring>
#include <NTL/ZZ_pX.h>
#include "kzg.h"
#include "util.h"

using namespace std;
using namespace NTL;

int main(int argc, char *argv[]) {
  /* ----- setup ------ */
  string data = "hello there my name is bob";
  KZG kzg1(data.size());
  kzg1.export_setup();
  
  KZG kzg("kzg_public");
  
  /* ----- commit ------ */
  KZGPolynomial P = KZG::create_polynomial_from_string(data, 0);
  KZGCommitment commit = kzg.commit(P);
  
  /* ----- create proof------ */
  KZGProof hello_proof = kzg.create_proof(P, 0, strlen("hello"));
  KZGProof name_proof = kzg.create_proof(P, strlen("hello there my "), strlen("name is"));
  KZGProof bob_proof = kzg.create_proof(P, strlen("hello there my name is "), strlen("bob"));
  
  /* ------ verify ------- */
  vector<KZGEvaluationPoint> verify;
  
  verify = KZG::create_evaluation_points_from_string("hello", 0);
  if (kzg.verify(commit, hello_proof, verify)) cout << "verified: hello" << endl;
  verify.clear();
  
  verify = KZG::create_evaluation_points_from_string("name is", strlen("hello there my "));
  if (kzg.verify(commit, name_proof, verify)) cout << "verified: name is" << endl;
  verify.clear();
  
  verify = KZG::create_evaluation_points_from_string("bob", strlen("hello there my name is "));
  if (kzg.verify(commit, bob_proof, verify)) cout << "verified: bob" << endl;
  verify.clear();
  
  verify = KZG::create_evaluation_points_from_string("alice", strlen("hello there my name is "));
  if (!kzg.verify(commit, bob_proof, verify)) cout << "verified: not alice" << endl;
  verify.clear();
  
  return 0;
}

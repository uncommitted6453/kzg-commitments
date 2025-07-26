#include <iostream>
#include <vector>
#include "kzg.h"
#include "util.h"

using namespace std;
using namespace NTL;

int main() {
  KZG kzg(32);
  
  vector<int> data = { 1, 3*3, 3*3*3, 3*3*3 };
  vector<pair<ZZ_p, ZZ_p>> points  = enumerate(data);
  
  ZZ_pX P = polyfit(points);
  SetCoeff(P, 0, 1);
  SetCoeff(P, 1, 2);
  SetCoeff(P, 2, 3);
  SetCoeff(P, 3, 4);
  
  ECP commit = kzg.commit(P);
  
  vector<int> X = { 1, 2 };
  ECP proof = kzg.multi_proof(P, X);
  
  vector<pair<ZZ_p, ZZ_p>> verify = {
    { conv<ZZ_p>(1), eval(P, conv<ZZ_p>(1)) },
    { conv<ZZ_p>(2), eval(P, conv<ZZ_p>(2)) }
  };
  kzg.multi_verify(commit, proof, verify);
  
  // ZZ_p x = conv<ZZ_p>(3);
  // ECP proof = kzg.single_proof(P, x);
  // kzg.single_verify(commit, proof, x, eval(P, x));
  
  return 0;
}

#include <iostream>
#include "kzg.h"

using namespace std;
using namespace NTL;

int main() {
  KZG kzg(32);
  
  ZZ_pX P;
  SetCoeff(P, 0, 2);
  SetCoeff(P, 1, -3);
  SetCoeff(P, 2, 4);
  
  ZZ_p x = conv<ZZ_p>(3);
  
  ECP commit = kzg.commit(P);
  ECP proof = kzg.single_proof(P, x);
  
  kzg.single_verify(commit, proof, x, eval(P, x));
  
  return 0;
}

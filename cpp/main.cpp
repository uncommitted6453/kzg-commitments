#include <iostream>
#include <vector>
#include "kzg.h"
#include "util.h"

using namespace std;
using namespace NTL;

int main() {
  KZG kzg(32);
  
  vector<int> data = { 11, 4, 2, 7, 1, 10 };
  vector<pair<ZZ_p, ZZ_p>> points  = enumerate(data);
  ZZ_pX P = polyfit(points);
  
  ECP commit = kzg.commit(P);
  
  vector<int> X = { 1, 2 };
  ECP proof = kzg.multi_proof(P, X);
  vector<pair<ZZ_p, ZZ_p>> verify = {
    { conv<ZZ_p>(1), eval(P, conv<ZZ_p>(1)) },
    { conv<ZZ_p>(2), eval(P, conv<ZZ_p>(2)) }
  };
  kzg.multi_verify(commit, proof, verify);
  
  return 0;
}

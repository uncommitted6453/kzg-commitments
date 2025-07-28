#ifndef KZG_H
#define KZG_H

#include <vector>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>
#include <NTL/ZZX.h>

using namespace std;
using namespace BN158;
using namespace NTL;

class KZG {
private:
  std::vector<ECP> _G1;
  std::vector<ECP2> _G2;
  
  ECP polyeval_G1(const ZZ_pX& P);
  ECP2 polyeval_G2(const ZZ_pX& P);

public:
  KZG(int num_coeff);
  
  ECP commit(const ZZ_pX& P);
  ECP create_proof(const ZZ_pX &P, int offset, int length);
  bool verify(ECP& commit, ECP& proof, std::vector<pair<ZZ_p, ZZ_p>>& points);
};

#endif

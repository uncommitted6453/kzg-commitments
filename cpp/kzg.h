#ifndef KZG_H
#define KZG_H

#include <vector>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>
#include <NTL/ZZX.h>

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
  ECP single_proof(const ZZ_pX &P, const ZZ_p& x);
  void single_verify(ECP& commit, ECP& proof, const ZZ_p& x, const ZZ_p& y);
};

#endif

#include <iostream>
#include <vector>
#include <fp_BN158.h>
#include <pair_BN158.h>
#include <fp12_BN158.h>
#include <big_B160_56.h>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>
#include <NTL/ZZX.h>

using namespace std;
using namespace BN158;
using namespace B160_56;
using namespace NTL;

void zz_to_big(BIG big, const ZZ& value) {
  unsigned char data[MODBYTES_B160_56];
  BytesFromZZ(data, value, MODBYTES_B160_56);
  
  int a = 0;
  int b = MODBYTES_B160_56 - 1;
  
  do
    swap<unsigned char>(data[a], data[b]);
  while (a++ < b--);
  
  BIG_fromBytesLen(big, (char*) data, MODBYTES_B160_56);
}

ZZ big_to_zz(const BIG big) {
  unsigned char data[MODBYTES_B160_56];
  BIG_toBytes((char*) data, (int64_t*) big);
  
  int a = 0;
  int b = MODBYTES_B160_56 - 1;
  
  do
    swap<unsigned char>(data[a], data[b]);
  while (a++ < b--);
  
  ZZ res;
  ZZFromBytes(res, data, MODBYTES_B160_56);
  
  return res;
}

class KZG {
private:
  vector<ECP> _G1;
  vector<ECP2> _G2;

public:
  KZG(int num_coeff);
  ECP polyeval_G1(const ZZ_pX& P);
  ECP2 polyeval_G2(const ZZ_pX& P);
  
  ECP single_proof(const ZZ_pX &P, const ZZ_p& x);
  void single_verify(ECP& commit, ECP& proof, const ZZ_p& x, const ZZ_p& y);
};

KZG::KZG(int num_coeff) {
  ZZ z = big_to_zz(CURVE_Order);
  ZZ_p::init(z);
  
  FP secret;
  FP_from_int(&secret, 42);
  
  BIG BIG_i;
  BIG_zero(BIG_i);
  
  for (int i = 0; i < num_coeff; i++) {
    FP s_i;
    BIG BIG_s_i;
    
    FP_pow(&s_i, &secret, BIG_i);
    FP_redc(BIG_s_i, &s_i);
    
    ECP G1_s_i;
    ECP_generator(&G1_s_i);
    ECP_mul(&G1_s_i, BIG_s_i);
    _G1.push_back(G1_s_i);
    
    ECP2 G2_s_i;
    ECP2_generator(&G2_s_i);
    ECP2_mul(&G2_s_i, BIG_s_i);
    _G2.push_back(G2_s_i);
    
    BIG_inc(BIG_i, 1);
  }
}

ECP KZG::polyeval_G1(const ZZ_pX& P) {
  BIG coeff_i;
  zz_to_big(coeff_i, rep(P[0]));
  
  ECP res;
  ECP_copy(&res, &_G1[0]);
  ECP_mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    zz_to_big(coeff_i, rep(P[i]));
    
    ECP term;
    ECP_copy(&term, &_G1[i]);
    ECP_mul(&term, coeff_i);
    
    ECP_add(&res, &term);
  }
  
  return res;
}

ECP2 KZG::polyeval_G2(const ZZ_pX& P) {
  BIG coeff_i;
  zz_to_big(coeff_i, rep(P[0]));
  
  ECP2 res;
  ECP2_copy(&res, &_G2[0]);
  ECP2_mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    zz_to_big(coeff_i, rep(P[i]));
    
    ECP2 term;
    ECP2_copy(&term, &_G2[i]);
    ECP2_mul(&term, coeff_i);
    
    ECP2_add(&res, &term);
  }
  
  return res;
}

ECP KZG::single_proof(const ZZ_pX &P, const ZZ_p& x) {
  ZZ_p y = eval(P, x);
  
  ZZ_pX I;
  SetCoeff(I, 0, y);
  
  ZZ_pX Z;
  SetCoeff(Z, 0, -x);
  SetCoeff(Z, 1, 1);
  
  ZZ_pX q = (P - I) / Z;
  
  return polyeval_G1(q);
}

void KZG::single_verify(ECP& commit, ECP& proof, const ZZ_p& z, const ZZ_p& y) {
  BIG BIG_z;
  zz_to_big(BIG_z, rep(z));
  
  BIG BIG_y;
  zz_to_big(BIG_y, rep(y));
  
  ECP2 p1;
  ECP2_copy(&p1, &_G2[0]);
  ECP2_mul(&p1, BIG_z);
  ECP2_neg(&p1);
  ECP2_add(&p1, &_G2[1]);
  
  FP12 v1;
  PAIR_ate(&v1, &p1, &proof);
  PAIR_fexp(&v1);
  
  ECP p2;
  ECP_copy(&p2, &_G1[0]);
  ECP_mul(&p2, BIG_y);
  ECP_neg(&p2);
  ECP_add(&p2, &commit);
  
  FP12 v2;
  PAIR_ate(&v2, &_G2[0], &p2);
  PAIR_fexp(&v2);
  
  FP12_output(&v1); cout << endl;
  FP12_output(&v2); cout << endl;
}

int main() {
  KZG kzg(32);
  
  ZZ_pX P;
  SetCoeff(P, 0, 2);
  SetCoeff(P, 1, -3);
  SetCoeff(P, 2, 4);
  
  ZZ_p x = conv<ZZ_p>(3);
  
  ECP commit = kzg.polyeval_G1(P);
  ECP proof = kzg.single_proof(P, x);
  
  kzg.single_verify(commit, proof, x, eval(P, x));
  
  return 0;
}

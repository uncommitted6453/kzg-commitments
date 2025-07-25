#include "kzg.h"

#include <fp12_BN158.h>
#include <pair_BN158.h>
#include <big_B160_56.h>
#include "util.h"

using namespace std;
using namespace BN158;
using namespace B160_56;
using namespace NTL;

KZG::KZG(int num_coeff) {
  ZZ z = ZZ_from_BIG(CURVE_Order);
  ZZ_p::init(z);
  
  ZZ_p s = conv<ZZ_p>("12345678");
  
  BIG BIG_i;
  BIG_zero(BIG_i);
  
  for (int i = 0; i < num_coeff; i++) {
    ZZ_p s_i = power(s, i);
    
    BIG BIG_s_i;
    BIG_from_ZZ(BIG_s_i, rep(s_i));
    
    ECP G1_s_i;
    ECP_generator(&G1_s_i);
    PAIR_G1mul(&G1_s_i, BIG_s_i);
    _G1.push_back(G1_s_i);
    
    ECP2 G2_s_i;
    ECP2_generator(&G2_s_i);
    PAIR_G2mul(&G2_s_i, BIG_s_i);
    _G2.push_back(G2_s_i);
    
    BIG_inc(BIG_i, 1);
  }
}

ECP KZG::commit(const ZZ_pX& P) {
  return polyeval_G1(P);
}

ECP KZG::polyeval_G1(const ZZ_pX& P) {
  BIG coeff_i;
  BIG_from_ZZ(coeff_i, rep(P[0]));
  
  ECP res;
  ECP_copy(&res, &_G1[0]);
  PAIR_G1mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    BIG_from_ZZ(coeff_i, rep(P[i]));
    
    ECP term;
    ECP_copy(&term, &_G1[i]);
    PAIR_G1mul(&term, coeff_i);
    
    ECP_add(&res, &term);
  }
  
  return res;
}

ECP2 KZG::polyeval_G2(const ZZ_pX& P) {
  BIG coeff_i;
  BIG_from_ZZ(coeff_i, rep(P[0]));
  
  ECP2 res;
  ECP2_copy(&res, &_G2[0]);
  PAIR_G2mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    BIG_from_ZZ(coeff_i, rep(P[i]));
    
    ECP2 term;
    ECP2_copy(&term, &_G2[i]);
    PAIR_G2mul(&term, coeff_i);
    
    ECP2_add(&res, &term);
  }
  
  return res;
}

ECP KZG::create_proof(const ZZ_pX &P, int offset, int length) {
  vector<pair<ZZ_p, ZZ_p>> points;
  
  for (int i = offset; i < offset + length; i++) {
    ZZ_p ZZ_x, ZZ_y;
    ZZ_x = i;
    ZZ_y = eval(P, ZZ_x);
    points.push_back({ ZZ_x, ZZ_y });
  }
  
  ZZ_pX I = polyfit(points);
  ZZ_pX Z = from_linear_roots(points);
  ZZ_pX q = (P - I) / Z;
  
  return polyeval_G1(q);
}

bool KZG::verify(ECP& commit, ECP& proof, std::vector<pair<ZZ_p, ZZ_p>>& points) {
  ZZ_pX I = polyfit(points);
  ZZ_pX Z = from_linear_roots(points);
  
  ECP2 p1 = polyeval_G2(Z);
  FP12 v1;
  PAIR_ate(&v1, &p1, &proof);
  PAIR_fexp(&v1);
  
  ECP p2 = polyeval_G1(I);
  ECP_neg(&p2);
  ECP_add(&p2, &commit);
  FP12 v2;
  PAIR_ate(&v2, &_G2[0], &p2);
  PAIR_fexp(&v2);
  
  return FP12_equals(&v1, &v2);
}

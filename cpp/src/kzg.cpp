#include "kzg.h"

#include <fp12_BN158.h>
#include <pair_BN158.h>
#include <big_B160_56.h>
#include <fstream>
#include <cstdint>
#include "util.h"

using namespace std;
using namespace BN158;
using namespace B160_56;
using namespace NTL;

namespace kzg {

public_params trusted_setup(int num_coeff) {
  public_params setup;
  
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
    setup._G1.push_back(G1_s_i);
    
    ECP2 G2_s_i;
    ECP2_generator(&G2_s_i);
    PAIR_G2mul(&G2_s_i, BIG_s_i);
    setup._G2.push_back(G2_s_i);
    
    BIG_inc(BIG_i, 1);
  }

  return setup;
}

/**
 *  this function will return an empty setup on fail.
 */
public_params load_params_file(const std::string& filename) {
  public_params setup;
  
  ZZ z = ZZ_from_BIG(CURVE_Order);
  ZZ_p::init(z);
  
  constexpr size_t G1_OCTET_SIZE = 2 * MODBYTES_B160_56 + 1;
  constexpr size_t G2_OCTET_SIZE = 4 * MODBYTES_B160_56 + 1;
  
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    return setup;
  }
  
  uint64_t num_coeffs;
  file.read(reinterpret_cast<char*>(&num_coeffs), sizeof(num_coeffs));
  
  setup._G1.reserve(static_cast<size_t>(num_coeffs));
  setup._G2.reserve(static_cast<size_t>(num_coeffs));
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    uint32_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    
    char buffer[G1_OCTET_SIZE];
    file.read(buffer, len);
    
    octet oct = {static_cast<int>(len), G1_OCTET_SIZE, buffer};
    ECP point;
    if (ECP_fromOctet(&point, &oct)) {
      setup._G1.push_back(point);
    } else {
      setup._G1.clear();
      setup._G2.clear();
      file.close();
      return setup;
    }
  }
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    uint32_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    
    char buffer[G2_OCTET_SIZE];
    file.read(buffer, len);
    
    octet oct = {static_cast<int>(len), G2_OCTET_SIZE, buffer};
    ECP2 point;
    if (ECP2_fromOctet(&point, &oct)) {
      setup._G2.push_back(point);
    } else {
      setup._G1.clear();
      setup._G2.clear();
      file.close();
      return setup;
    }
  }
  
  file.close();
  
  return setup;
}

ECP commit(const public_params& setup, const ZZ_pX& P) {
  return polyeval_G1(setup, P);
}

ECP polyeval_G1(const public_params& setup, const ZZ_pX& P) {
  BIG coeff_i;
  BIG_from_ZZ(coeff_i, rep(P[0]));
  
  ECP res;
  ECP_copy(&res, &setup._G1[0]);
  PAIR_G1mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    BIG_from_ZZ(coeff_i, rep(P[i]));
    
    ECP term;
    ECP_copy(&term, &setup._G1[i]);
    PAIR_G1mul(&term, coeff_i);
    
    ECP_add(&res, &term);
  }
  
  return res;
}

ECP2 polyeval_G2(const public_params& setup, const ZZ_pX& P) {
  BIG coeff_i;
  BIG_from_ZZ(coeff_i, rep(P[0]));
  
  ECP2 res;
  ECP2_copy(&res, &setup._G2[0]);
  PAIR_G2mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    BIG_from_ZZ(coeff_i, rep(P[i]));
    
    ECP2 term;
    ECP2_copy(&term, &setup._G2[i]);
    PAIR_G2mul(&term, coeff_i);
    
    ECP2_add(&res, &term);
  }
  
  return res;
}

ECP create_proof(const public_params& setup, const ZZ_pX &P, int offset, int length) {
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
  
  return polyeval_G1(setup, q);
}

bool verify(const public_params& setup, ECP& commit, ECP& proof, std::vector<pair<ZZ_p, ZZ_p>>& points) {
  ZZ_pX I = polyfit(points);
  ZZ_pX Z = from_linear_roots(points);
  
  ECP2 p1 = polyeval_G2(setup, Z);
  FP12 v1;
  PAIR_ate(&v1, &p1, &proof);
  PAIR_fexp(&v1);
  
  ECP p2 = polyeval_G1(setup, I);
  ECP_neg(&p2);
  ECP_add(&p2, &commit);
  FP12 v2;
  PAIR_ate(&v2, &setup._G2[0], &p2);
  PAIR_fexp(&v2);
  
  return FP12_equals(&v1, &v2);
}

void export_params_file(const public_params& setup, const std::string& filename) {
  std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    return;
  }
  
  constexpr size_t G1_OCTET_SIZE = 2 * MODBYTES_B160_56 + 1;
  constexpr size_t G2_OCTET_SIZE = 4 * MODBYTES_B160_56 + 1;
  
  uint64_t num_coeffs = static_cast<uint64_t>(setup._G1.size());
  file.write(reinterpret_cast<const char*>(&num_coeffs), sizeof(num_coeffs));
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    char buffer[G1_OCTET_SIZE];
    octet oct = {0, G1_OCTET_SIZE, buffer};
    ECP_toOctet(&oct, &setup._G1[i], false);
    
    uint32_t len = static_cast<uint32_t>(oct.len);
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(oct.val, oct.len);
  }
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    char buffer[G2_OCTET_SIZE];
    octet oct = {0, G2_OCTET_SIZE, buffer};
    ECP2_toOctet(&oct, &setup._G2[i], false);

    uint32_t len = static_cast<uint32_t>(oct.len);
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(oct.val, oct.len);
  }
  
  file.close();
}

}

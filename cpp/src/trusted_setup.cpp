#include "kzg.h"

#include <fp12_BN158.h>
#include <pair_BN158.h>
#include <big_B160_56.h>
#include <fstream>
#include <cstdint>
#include <thread>
#include <vector>
#include <functional>
#include "util.h"

using namespace std;
using namespace BN158;
using namespace B160_56;
using namespace NTL;
using namespace core;

void kzg::trusted_setup::generate_elements_range(
  int start, int end, BIG s_i, BIG s
) {
  ECP G1_s_i;
  ECP_generator(&G1_s_i);
  PAIR_G1mul(&G1_s_i, s_i);
  _G1[start] = G1_s_i;

  ECP2 G2_s_i;
  ECP2_generator(&G2_s_i);
  PAIR_G2mul(&G2_s_i, s_i);
  _G2[start] = G2_s_i;

  for (int i = start + 1; i < end; i++) {
    PAIR_G1mul(&G1_s_i, s);
    _G1[i] = G1_s_i;
    
    PAIR_G2mul(&G2_s_i, s);
    _G2[i] = G2_s_i;
  }
}

kzg::trusted_setup::trusted_setup(int num_coeff) {
  ZZ z = ZZ_from_BIG(CURVE_Order);
  ZZ_p::init(z);
  
  BIG BIG_s;
  generate_random_BIG(BIG_s);
  ZZ_p s = conv<ZZ_p>(ZZ_from_BIG(BIG_s));

  _G1.resize(num_coeff);
  _G2.resize(num_coeff);

  unsigned int num_threads = std::thread::hardware_concurrency();
  if (num_threads == 0) {
    num_threads = 4;
  }
  
  int elements_per_thread = num_coeff / num_threads;
  int remaining_elements = num_coeff % num_threads;
  
  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  int start = 0;
  for (unsigned int t = 0; t < num_threads; t++) {
    // distribute remaining elements 1 each to first remaining_elements threads
    int num_elements_to_handle = elements_per_thread;
    if (t < remaining_elements) {
      num_elements_to_handle += 1;
    }

    int end = start + num_elements_to_handle;
    if (start < num_coeff) {
      BIG BIG_s_i;
      ZZ_p s_i = power(s, start);
      BIG_from_ZZ(BIG_s_i, rep(s_i));

      threads.push_back(std::thread(
        &kzg::trusted_setup::generate_elements_range, 
        this, start, end, BIG_s_i, BIG_s
      ));
    }
    start = end;
  }
  
  for (auto& thread : threads) {
    thread.join();
  }
}

kzg::trusted_setup::trusted_setup(const std::string& filename) {
  ZZ z = ZZ_from_BIG(CURVE_Order);
  ZZ_p::init(z);
  
  constexpr size_t G1_OCTET_SIZE = 2 * MODBYTES_B160_56 + 1;
  constexpr size_t G2_OCTET_SIZE = 4 * MODBYTES_B160_56 + 1;
  
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "failed to import" << std::endl;
    return;
  }
  
  uint64_t num_coeffs;
  file.read(reinterpret_cast<char*>(&num_coeffs), sizeof(num_coeffs));
  
  _G1.reserve(static_cast<size_t>(num_coeffs));
  _G2.reserve(static_cast<size_t>(num_coeffs));
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    uint32_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    
    char buffer[G1_OCTET_SIZE];
    file.read(buffer, len);
    
    octet oct = {static_cast<int>(len), G1_OCTET_SIZE, buffer};
    ECP point;
    if (ECP_fromOctet(&point, &oct)) {
      _G1.push_back(point);
    } else {
      std::cerr << "point at index " << i << " is invalid" << std::endl;
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
      _G2.push_back(point);
    } else {
      std::cerr << "point at index " << i << " is invalid" << std::endl;
    }
  }
  
  file.close();
  std::cout << "loaded group elements from " << filename << " with num_coeffs=" << num_coeffs << "" << std::endl;
}

kzg::commit kzg::trusted_setup::create_commit(const kzg::poly& poly) {
  return kzg::commit(polyeval_G1(poly.get_poly()));
}

bool kzg::trusted_setup::verify_commit(kzg::commit& commit, const kzg::poly& poly) {
  kzg::commit expected_commit = create_commit(poly);
  return ECP_equals(&commit.get_curve_point(), &expected_commit.get_curve_point());
}

ECP kzg::trusted_setup::polyeval_G1(const ZZ_pX& P) {
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

ECP2 kzg::trusted_setup::polyeval_G2(const ZZ_pX& P) {
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

kzg::proof kzg::trusted_setup::create_proof(const kzg::poly& poly, int offset, int length) {
  const ZZ_pX& P = poly.get_poly();
  
  vector<pair<ZZ_p, ZZ_p>> points;
  evaluate_polynomial_points(points, P, offset, length);
  
  ZZ_pX I, Z;
  linear_roots_and_polyfit(I, Z, points);
  ZZ_pX q = (P - I) / Z;
  
  return kzg::proof(polyeval_G1(q));
}

bool kzg::trusted_setup::verify_proof(kzg::commit& commit, kzg::proof& proof, kzg::blob& expected_data) {
  vector<pair<ZZ_p, ZZ_p>>& points = expected_data.get_data();
  
  ZZ_pX I, Z;
  linear_roots_and_polyfit(I, Z, points);
  
  ECP2 p1 = polyeval_G2(Z);
  FP12 v1;
  PAIR_ate(&v1, &p1, &proof.get_curve_point());
  PAIR_fexp(&v1);
  
  ECP p2 = polyeval_G1(I);
  ECP_neg(&p2);
  ECP_add(&p2, &commit.get_curve_point());
  FP12 v2;
  PAIR_ate(&v2, &_G2[0], &p2);
  PAIR_fexp(&v2);
  
  return FP12_equals(&v1, &v2);
}

void kzg::trusted_setup::export_setup(const std::string& filename) {
  std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << "failed to export" << std::endl;
    return;
  }
  
  constexpr size_t G1_OCTET_SIZE = 2 * MODBYTES_B160_56 + 1;
  constexpr size_t G2_OCTET_SIZE = 4 * MODBYTES_B160_56 + 1;
  
  uint64_t num_coeffs = static_cast<uint64_t>(_G1.size());
  file.write(reinterpret_cast<const char*>(&num_coeffs), sizeof(num_coeffs));
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    char buffer[G1_OCTET_SIZE];
    octet oct = {0, G1_OCTET_SIZE, buffer};
    ECP_toOctet(&oct, &_G1[i], false);
    // std::cout << oct.len << std::endl;
    
    uint32_t len = static_cast<uint32_t>(oct.len);
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(oct.val, oct.len);
  }
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    char buffer[G2_OCTET_SIZE];
    octet oct = {0, G2_OCTET_SIZE, buffer};
    ECP2_toOctet(&oct, &_G2[i], false);

    uint32_t len = static_cast<uint32_t>(oct.len);
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(oct.val, oct.len);
  }
  
  file.close();
  std::cout << "exported group elements to " << filename << " with num_coeffs=" << num_coeffs << "" << std::endl;
}


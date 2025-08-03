#include "kzg.h"

#include <fp12_BN158.h>
#include <pair_BN158.h>
#include <big_B160_56.h>
#include <fstream>
#include <memory>
#include <iostream>
#include "util.h"

using namespace std;
using namespace BN158;
using namespace B160_56;
using namespace NTL;
using namespace core;

static KZGPolynomial serialize_ZZ_pX(const ZZ_pX& poly) {
  std::vector<uint8_t> result;
  
  long degree = deg(poly);
  result.insert(result.end(), 
                reinterpret_cast<const uint8_t*>(&degree), 
                reinterpret_cast<const uint8_t*>(&degree) + sizeof(degree));
  
  for (long i = 0; i <= degree; i++) {
    const ZZ& coeff_zz = rep(coeff(poly, i));
    
    long num_bytes = NumBytes(coeff_zz);
    
    result.insert(result.end(), 
                  reinterpret_cast<const uint8_t*>(&num_bytes), 
                  reinterpret_cast<const uint8_t*>(&num_bytes) + sizeof(num_bytes));
    
    if (num_bytes > 0) {
      std::vector<uint8_t> coeff_bytes(num_bytes);
      BytesFromZZ(coeff_bytes.data(), coeff_zz, num_bytes);
      result.insert(result.end(), coeff_bytes.begin(), coeff_bytes.end());
    }
  }
  
  return result;
}

static ZZ_pX deserialize_ZZ_pX(const KZGPolynomial& bytes) {
  size_t offset = 0;
  
  long degree;
  std::memcpy(&degree, bytes.data() + offset, sizeof(degree));
  offset += sizeof(degree);
  
  ZZ_pX poly;
  if (degree >= 0) {
    poly.SetLength(degree + 1);
    
    for (long i = 0; i <= degree; i++) {
      long num_bytes;
      std::memcpy(&num_bytes, bytes.data() + offset, sizeof(num_bytes));
      offset += sizeof(num_bytes);
      
      if (num_bytes > 0) {
        ZZ coeff_zz;
        ZZFromBytes(coeff_zz, bytes.data() + offset, num_bytes);
        offset += num_bytes;
        
        conv(poly[i], coeff_zz);
      } else {
        clear(poly[i]);
      }
    }
  }
  
  poly.normalize();
  return poly;
}

static KZGCommitment serialize_ECP(const ECP& point) {
  constexpr size_t G1_OCTET_SIZE = 2 * MODBYTES_B160_56 + 1;
  char buffer[G1_OCTET_SIZE];
  octet oct = {0, G1_OCTET_SIZE, buffer};
  ECP_toOctet(&oct, const_cast<ECP*>(&point), false);
  
  std::vector<uint8_t> result;
  uint32_t len = static_cast<uint32_t>(oct.len);
  result.insert(result.end(), 
                reinterpret_cast<const uint8_t*>(&len), 
                reinterpret_cast<const uint8_t*>(&len) + sizeof(len));
  result.insert(result.end(), 
                reinterpret_cast<const uint8_t*>(oct.val), 
                reinterpret_cast<const uint8_t*>(oct.val) + oct.len);
  
  return result;
}

static ECP deserialize_ECP(const KZGCommitment& bytes) {
  constexpr size_t G1_OCTET_SIZE = 2 * MODBYTES_B160_56 + 1;
  
  uint32_t len;
  std::memcpy(&len, bytes.data(), sizeof(len));
  
  char buffer[G1_OCTET_SIZE];
  std::memcpy(buffer, bytes.data() + sizeof(len), len);
  
  octet oct = {static_cast<int>(len), G1_OCTET_SIZE, buffer};
  ECP point;
  if (!ECP_fromOctet(&point, &oct)) {
    std::cerr << "failed to deserialize ECP point" << std::endl;
  }
  
  return point;
}

static KZGFieldElement serialize_ZZ_p(const ZZ_p& element) {
  const ZZ& element_zz = rep(element);
  long num_bytes = NumBytes(element_zz);
  
  std::vector<uint8_t> result;
  result.insert(result.end(), 
                reinterpret_cast<const uint8_t*>(&num_bytes), 
                reinterpret_cast<const uint8_t*>(&num_bytes) + sizeof(num_bytes));
  
  if (num_bytes > 0) {
    std::vector<uint8_t> element_bytes(num_bytes);
    BytesFromZZ(element_bytes.data(), element_zz, num_bytes);
    result.insert(result.end(), element_bytes.begin(), element_bytes.end());
  }
  
  return result;
}

static ZZ_p deserialize_ZZ_p(const KZGFieldElement& bytes) {
  long num_bytes;
  std::memcpy(&num_bytes, bytes.data(), sizeof(num_bytes));
  
  ZZ_p element;
  if (num_bytes > 0) {
    ZZ element_zz;
    ZZFromBytes(element_zz, bytes.data() + sizeof(num_bytes), num_bytes);
    conv(element, element_zz);
  } else {
    clear(element);
  }
  
  return element;
}

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

KZG::KZG(const std::string& filename) {
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

KZGCommitment KZG::commit(const KZGPolynomial& polynomial) {
  ZZ_pX P = deserialize_ZZ_pX(polynomial);
  ECP commitment = polyeval_G1(P);
  return serialize_ECP(commitment);
}

KZGProof KZG::create_proof(const KZGPolynomial& polynomial, int offset, int length) {
  ZZ_pX P = deserialize_ZZ_pX(polynomial);
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
  
  ECP proof = polyeval_G1(q);
  KZGProof result = serialize_ECP(proof);
  return result;
}

bool KZG::verify(const KZGCommitment& commitment, const KZGProof& proof, 
                 const std::vector<KZGEvaluationPoint>& points) {
  ECP commit_point = deserialize_ECP(commitment);
  ECP proof_point = deserialize_ECP(proof);
  
  vector<pair<ZZ_p, ZZ_p>> internal_points;
  for (const auto& point : points) {
    ZZ_p x = deserialize_ZZ_p(point.first);
    ZZ_p y = deserialize_ZZ_p(point.second);
    internal_points.push_back({x, y});
  }
  
  ZZ_pX I = polyfit(internal_points);
  ZZ_pX Z = from_linear_roots(internal_points);
  
  ECP2 p1 = polyeval_G2(Z);
  FP12 v1;
  PAIR_ate(&v1, &p1, &proof_point);
  PAIR_fexp(&v1);
  
  ECP p2 = polyeval_G1(I);
  ECP_neg(&p2);
  ECP_add(&p2, &commit_point);
  FP12 v2;
  PAIR_ate(&v2, &_G2[0], &p2);
  PAIR_fexp(&v2);
  
  return FP12_equals(&v1, &v2);
}

KZGPolynomial KZG::create_polynomial_from_string(const std::string& data, int offset) {
  vector<pair<ZZ_p, ZZ_p>> points;
  create_points_from_string(points, data, offset);
  ZZ_pX P = polyfit(points);
  return serialize_ZZ_pX(P);
}

std::vector<KZGEvaluationPoint> KZG::create_evaluation_points_from_string(const std::string& data, int offset) {
  vector<pair<ZZ_p, ZZ_p>> internal_points;
  create_points_from_string(internal_points, data, offset);
  
  std::vector<KZGEvaluationPoint> result;
  for (const auto& point : internal_points) {
    KZGFieldElement x_serialized = serialize_ZZ_p(point.first);
    KZGFieldElement y_serialized = serialize_ZZ_p(point.second);
    result.push_back({x_serialized, y_serialized});
  }
  
  return result;
}

void KZG::export_setup(const std::string& filename) {
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

#include "util.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <array>
#include <randapi.h>

#define FAST_MULTIEVAL_THRESHOLD 140

using namespace std;
using namespace B160_56;
using namespace BN158;
using namespace NTL;
using namespace core;

static ZZ_pX build_linear_roots_tree(
  vector<ZZ_pX>& linear_roots,
  vector<pair<ZZ_p, ZZ_p>>& points,
  int lo, int hi
);

static int multieval_R(
  vector<ZZ_p>& res,
  const vector<ZZ_pX>& linear_roots,
  const ZZ_pX& f,
  int lo, int hi, int idx
);

static ZZ_pX polyfit_R(
  vector<pair<ZZ_p, ZZ_p>>& points,
  const vector<ZZ_pX>& linear_roots,
  int lo, int hi, int& idx
);

void BIG_from_ZZ(BIG big, const ZZ& value) {
  unsigned char data[MODBYTES_B160_56];
  BytesFromZZ(data, value, MODBYTES_B160_56);
  
  int a = 0;
  int b = MODBYTES_B160_56 - 1;
  
  while (a < b) {
    swap<unsigned char>(data[a++], data[b--]);
  }
  
  BIG_fromBytesLen(big, (char*) data, MODBYTES_B160_56);
}

ZZ ZZ_from_BIG(const BIG big) {
  unsigned char data[MODBYTES_B160_56];
  BIG_toBytes((char*) data, (int64_t*) big);
  
  int a = 0;
  int b = MODBYTES_B160_56 - 1;
  
  while (a < b) {
    swap<unsigned char>(data[a++], data[b--]);
  }
  
  ZZ res;
  ZZFromBytes(res, data, MODBYTES_B160_56);
  
  return res;
}

void generate_random_BIG(BIG& random) {
  csprng rng;
  std::random_device gen;
  std::array<unsigned char, 32> seed_bytes;
  for (int i = 0; i < 32; i++) {
    seed_bytes[i] = static_cast<unsigned char>(gen());
  }
  RAND_seed(&rng, seed_bytes.size(), reinterpret_cast<char*>(seed_bytes.data()));

  BIG curve_order_copy;
  BIG_rcopy(curve_order_copy, CURVE_Order);
  
  BIG_randomnum(random, curve_order_copy, &rng);
  RAND_clean(&rng);
}

std::vector<uint8_t> serialize_ECP(const ECP& point) {
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

ECP deserialize_ECP(const std::vector<uint8_t>& bytes) {
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


vector<uint8_t> serialize_ZZ_pX(const ZZ_pX& poly) {
  std::vector<uint8_t> result;
  
  long degree = deg(poly);
  result.insert(result.end(), 
                reinterpret_cast<const uint8_t*>(&degree), 
                reinterpret_cast<const uint8_t*>(&degree) + sizeof(degree));
  
  for (long i = 0; i <= degree; i++) {
    const ZZ& coeff_zz = rep(coeff(poly, i));
    
    uint8_t num_bytes = (uint8_t) NumBytes(coeff_zz);
    result.push_back(num_bytes);
    
    if (num_bytes > 0) {
      uint8_t coeff_bytes[MODBYTES_B160_56];
      BytesFromZZ(coeff_bytes, coeff_zz, num_bytes);
      result.insert(result.end(), coeff_bytes, coeff_bytes + num_bytes);
    }
  }
  
  return result;
}

ZZ_pX deserialize_ZZ_pX(const vector<uint8_t>& bytes) {
  size_t offset = 0;
  
  long degree;
  std::memcpy(&degree, bytes.data() + offset, sizeof(degree));
  offset += sizeof(degree);
  
  ZZ_pX poly;
  if (degree >= 0) {
    poly.SetLength(degree + 1);
    
    for (long i = 0; i <= degree; i++) {
      uint8_t num_bytes = bytes[offset++];
      
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

void linear_roots_and_polyfit(ZZ_pX& result, ZZ_pX& linear_roots, vector<pair<ZZ_p, ZZ_p>>& points) {
  int idx = 0;
  vector<pair<ZZ_p, ZZ_p>> copy = points;
  vector<ZZ_pX> linear_roots_tree;
  linear_roots = build_linear_roots_tree(linear_roots_tree, points, 0, points.size() - 1);
  result = polyfit_R(copy, linear_roots_tree, 0, points.size() - 1, idx);
}

ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points) {
  ZZ_pX fitted_poly, linear_roots;
  linear_roots_and_polyfit(fitted_poly, linear_roots, points);
  return fitted_poly;
}

void evaluate_polynomial_points(vector<pair<ZZ_p, ZZ_p>>& points, const ZZ_pX& poly, int offset, int length) {
  if (length < FAST_MULTIEVAL_THRESHOLD) {
    for (int i = offset; i < offset + length; i++) {
      ZZ_p ZZ_x, ZZ_y;
      ZZ_x = i;
      ZZ_y = eval(poly, ZZ_x);
      points.push_back({ ZZ_x, ZZ_y });
    }
  } else {
    for (int i = offset; i <= offset + length; i++) {
      ZZ_p ZZ_x, ZZ_y;
      ZZ_x = i;
      ZZ_y = 0;
      points.push_back({ ZZ_x, ZZ_y });
    }
    
    vector<ZZ_pX> linear_roots_tree;
    build_linear_roots_tree(linear_roots_tree, points, 0, points.size() - 1);
    vector<ZZ_p> points_eval;
    multieval_R(points_eval, linear_roots_tree, poly, 0, points.size() - 1, 0);
    
    for (int i = 0; i < points.size(); i++) {
      points[i].second = points_eval[i];
    }
  }
}

static ZZ_pX polyfit_R(vector<pair<ZZ_p, ZZ_p>>& points, const vector<ZZ_pX>& linear_roots, int lo, int hi, int& idx) {
  if (lo == hi) {
    ZZ_pX f;
    SetCoeff(f, 0, points[lo].second);
    return f;
  }
  
  const ZZ_pX& Z_1 = linear_roots[linear_roots.size() - idx++ - 1];
  const ZZ_pX& Z_2 = linear_roots[linear_roots.size() - idx++ - 1];
  
  int mid = (lo + hi) / 2;
  
  if (mid - lo < FAST_MULTIEVAL_THRESHOLD) {
    for (int i = lo; i <= mid; i++)
      points[i].second /= eval(Z_2, points[i].first);
  } else {
    vector<ZZ_p> prod_eval;
    multieval_R(prod_eval, linear_roots, Z_2, lo, mid, idx);
    for (int i = 0; i < prod_eval.size(); i++)
      points[lo + i].second /= prod_eval[i];
  }
  ZZ_pX f_1 = polyfit_R(points, linear_roots, lo, mid, idx);
  
  if (hi - (mid + 1) < FAST_MULTIEVAL_THRESHOLD) {
    for (int i = mid + 1; i <= hi; i++)
      points[i].second /= eval(Z_1, points[i].first);
  } else {
    vector<ZZ_p> prod_eval;
    multieval_R(prod_eval, linear_roots, Z_1, mid + 1, hi, idx);
    for (int i = 0; i < prod_eval.size(); i++)
      points[mid + 1 + i].second /= prod_eval[i];
  }
  ZZ_pX f_2 = polyfit_R(points, linear_roots, mid + 1, hi, idx);
  
  return f_2 * Z_1 + f_1 * Z_2;
}

static int multieval_R(vector<ZZ_p>& res, const vector<ZZ_pX>& linear_roots, const ZZ_pX& f, int lo, int hi, int idx) {
  if (lo == hi) {
    res.push_back(f[0]);
    return idx;
  }
  
  const ZZ_pX& Z_1 = linear_roots[linear_roots.size() - idx++ - 1];
  const ZZ_pX& Z_2 = linear_roots[linear_roots.size() - idx++ - 1];
  
  ZZ_pX f_1 = f % Z_1;
  ZZ_pX f_2 = f % Z_2;
  
  int mid = (lo + hi) / 2;
  idx = multieval_R(res, linear_roots, f_1, lo, mid, idx);
  idx = multieval_R(res, linear_roots, f_2, mid + 1, hi, idx);
  
  return idx;
}

static ZZ_pX build_linear_roots_tree(vector<ZZ_pX>& linear_roots, vector<pair<ZZ_p, ZZ_p>>& points, int lo, int hi) {
  if (lo == hi) {
    ZZ_pX linear_root;
    SetCoeff(linear_root, 0, -points[lo].first);
    SetCoeff(linear_root, 1, 1);
    return linear_root;
  }
  
  int mid = (lo + hi) / 2;
  ZZ_pX Z_2 = build_linear_roots_tree(linear_roots, points, mid + 1, hi);
  ZZ_pX Z_1 = build_linear_roots_tree(linear_roots, points, lo, mid);
  linear_roots.push_back(Z_2);
  linear_roots.push_back(Z_1);
  
  return Z_1 * Z_2;
}

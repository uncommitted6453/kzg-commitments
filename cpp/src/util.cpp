#include "util.h"

#include <algorithm>
#include <cstring>
#include <iostream>

using namespace std;
using namespace B160_56;
using namespace BN158;
using namespace NTL;
using namespace core;

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

ZZ_pX build_linear_roots_tree(vector<ZZ_pX>& linear_roots, vector<pair<ZZ_p, ZZ_p>>& points, int lo, int hi) {
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

int multieval_R(vector<ZZ_p>& res, vector<ZZ_pX>& linear_roots, ZZ_pX& f, int lo, int hi, int idx) {
  if (lo == hi) {
    res.push_back(f[0]);
    return idx;
  }
  
  ZZ_pX& Z_1 = linear_roots[linear_roots.size() - idx++ - 1];
  ZZ_pX& Z_2 = linear_roots[linear_roots.size() - idx++ - 1];
  
  ZZ_pX f_1 = f % Z_1;
  ZZ_pX f_2 = f % Z_2;
  
  int mid = (lo + hi) / 2;
  idx = multieval_R(res, linear_roots, f_1, lo, mid, idx);
  idx = multieval_R(res, linear_roots, f_2, mid + 1, hi, idx);
  
  return idx;
}

ZZ_pX polyfit_R(vector<pair<ZZ_p, ZZ_p>>& points, vector<ZZ_pX>& linear_roots, int lo, int hi, int& idx) {
  if (lo == hi) {
    ZZ_pX f;
    SetCoeff(f, 0, points[lo].second);
    return f;
  }
  
  ZZ_pX& Z_1 = linear_roots[linear_roots.size() - idx++ - 1];
  ZZ_pX& Z_2 = linear_roots[linear_roots.size() - idx++ - 1];
  
  int mid = (lo + hi) / 2;
  
  if (mid - lo < 140) {
    for (int i = lo; i <= mid; i++)
      points[i].second /= eval(Z_2, points[i].first);
  } else {
    vector<ZZ_p> prod_eval;
    multieval_R(prod_eval, linear_roots, Z_2, lo, mid, idx);
    for (int i = 0; i < prod_eval.size(); i++)
      points[lo + i].second /= prod_eval[i];
  }
  ZZ_pX f_1 = polyfit_R(points, linear_roots, lo, mid, idx);
  
  if (hi - (mid + 1) < 140) {
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

ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points) {
  vector<pair<ZZ_p, ZZ_p>> copy = points;
  int idx = 0;
  vector<ZZ_pX> linear_roots;
  build_linear_roots_tree(linear_roots, points, 0, points.size() - 1);
  return polyfit_R(copy, linear_roots, 0, points.size() - 1, idx);
}

// ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points) {
//   ZZ_pX P;
//   
//   for (const pair<ZZ_p, ZZ_p>& A : points) {
//     ZZ_pX bottom;
//     SetCoeff(bottom, 0, 1);
//     
//     ZZ_pX top;
//     SetCoeff(top, 0, A.second);
//     
//     for (const pair<ZZ_p, ZZ_p>& B : points) {
//       if (A.first == B.first)
//         continue;
//       
//       ZZ_pX factor;
//       SetCoeff(factor, 0, -B.first);
//       SetCoeff(factor, 1, 1);
//       
//       top *= factor;
//       bottom *= A.first - B.first;
//     }
//     P += top / bottom;
//   }
//   
//   return P;
// }

ZZ_pX from_linear_roots(vector<pair<ZZ_p, ZZ_p>>& points) {
  ZZ_pX Z;
  SetCoeff(Z, 0, 1);
  
  for (const pair<ZZ_p, ZZ_p>& point : points) {
    ZZ_pX factor;
    SetCoeff(factor, 0, -point.first);
    SetCoeff(factor, 1, 1);
    Z *= factor;
  }
  
  return Z;
}

void create_points_from_string(vector<pair<ZZ_p, ZZ_p>>& res, string data, int offset) {
  for (int i = 0; i < data.size(); i++) {
    ZZ_p ZZ_x, ZZ_y;
    ZZ_x = i + offset;
    ZZ_y = data[i];
    res.push_back({ ZZ_x, ZZ_y });
  }
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

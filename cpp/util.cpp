#include "util.h"

#include <algorithm>

using namespace std;
using namespace B160_56;
using namespace NTL;

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

ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points) {
  ZZ_pX P;
  
  for (const pair<ZZ_p, ZZ_p>& A : points) {
    ZZ_pX bottom;
    SetCoeff(bottom, 0, 1);
    
    ZZ_pX top;
    SetCoeff(top, 0, A.second);
    
    for (const pair<ZZ_p, ZZ_p>& B : points) {
      if (A.first == B.first)
        continue;
      
      ZZ_pX factor;
      SetCoeff(factor, 0, -B.first);
      SetCoeff(factor, 1, 1);
      
      top *= factor;
      bottom *= A.first - B.first;
    }
    P += top / bottom;
  }
  
  return P;
}

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

extern void create_points_from_string(vector<pair<ZZ_p, ZZ_p>>& res, string data, int offset) {
  for (int i = 0; i < data.size(); i++) {
    ZZ_p ZZ_x, ZZ_y;
    ZZ_x = i + offset;
    ZZ_y = data[i];
    res.push_back({ ZZ_x, ZZ_y });
  }
}

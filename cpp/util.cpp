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
  
  do
    swap<unsigned char>(data[a], data[b]);
  while (a++ < b--);
  
  BIG_fromBytesLen(big, (char*) data, MODBYTES_B160_56);
}

ZZ ZZ_from_BIG(const BIG big) {
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

vector<pair<ZZ_p, ZZ_p>> enumerate(vector<int>& values) {
  vector<pair<ZZ_p, ZZ_p>> points;
  
  for (int i = 0; i < values.size(); i++) {
    ZZ_p x, y;
    x = i;
    y = values[i];
    points.push_back({ x, y });
  }
  
  return points;
}

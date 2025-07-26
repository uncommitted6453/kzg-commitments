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

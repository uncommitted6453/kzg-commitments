#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <big_B160_56.h>
#include <NTL/ZZX.h>

using namespace std;
using namespace B160_56;
using namespace NTL;

extern void BIG_from_ZZ(BIG big, const ZZ& value);
extern ZZ ZZ_from_BIG(const BIG big);
extern ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points);
extern vector<pair<ZZ_p, ZZ_p>> enumerate(vector<int>& data);

#endif

#ifndef UTIL_H
#define UTIL_H

#include <big_B160_56.h>
#include <NTL/ZZX.h>

using namespace B160_56;
using namespace NTL;

extern void BIG_from_ZZ(BIG big, const ZZ& value);
extern ZZ ZZ_from_BIG(const BIG big);

#endif

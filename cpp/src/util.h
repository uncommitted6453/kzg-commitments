#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <big_B160_56.h>
#include <NTL/ZZX.h>
#include <core.h>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>

using namespace std;
using namespace B160_56;
using namespace BN158;
using namespace NTL;
using namespace core;

void BIG_from_ZZ(BIG big, const ZZ& value);
ZZ ZZ_from_BIG(const BIG big);
ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points);
ZZ_pX from_linear_roots(vector<pair<ZZ_p, ZZ_p>>& points);
void create_points_from_string(vector<pair<ZZ_p, ZZ_p>>& res, string data, int offset);
std::vector<uint8_t> serialize_ECP(const ECP& point);
ECP deserialize_ECP(const std::vector<uint8_t>& bytes);

#endif

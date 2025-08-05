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

extern void BIG_from_ZZ(BIG big, const ZZ& value);
extern ZZ ZZ_from_BIG(const BIG big);
extern ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points);
extern void linear_roots_and_polyfit(ZZ_pX& result, ZZ_pX& linear_roots, vector<pair<ZZ_p, ZZ_p>>& points);
extern void evaluate_polynomial_points(vector<pair<ZZ_p, ZZ_p>>& points, const ZZ_pX& poly, int start, int length);
extern void generate_random_BIG(BIG& random);
extern std::vector<uint8_t> serialize_ECP(const ECP& point);
extern ECP deserialize_ECP(const std::vector<uint8_t>& bytes);

#endif

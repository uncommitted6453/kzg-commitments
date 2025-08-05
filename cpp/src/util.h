#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <NTL/ZZX.h>
#include "kzg_config.h"

using namespace std;
using namespace NTL;

extern void BIG_from_ZZ(BIG big, const ZZ& value);
extern ZZ ZZ_from_BIG(const BIG big);
extern ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points);
extern void linear_roots_and_polyfit(ZZ_pX& result, ZZ_pX& linear_roots, vector<pair<ZZ_p, ZZ_p>>& points);
extern void evaluate_polynomial_points(vector<pair<ZZ_p, ZZ_p>>& points, const ZZ_pX& poly, int start, int length);
extern void generate_random_BIG(BIG& random);
extern std::vector<uint8_t> serialize_ECP(const ECP& point);
extern ECP deserialize_ECP(const std::vector<uint8_t>& bytes);
extern std::vector<uint8_t> serialize_ZZ_pX(const ZZ_pX& poly);
extern ZZ_pX deserialize_ZZ_pX(const std::vector<uint8_t>& bytes);

#endif

#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <NTL/ZZX.h>
#include <kzg_config.h>

using namespace std;
using namespace NTL;

void BIG_from_ZZ(BIG big, const ZZ& value);
ZZ ZZ_from_BIG(const BIG big);
ZZ_pX polyfit(vector<pair<ZZ_p, ZZ_p>>& points);
void linear_roots_and_polyfit(ZZ_pX& result, ZZ_pX& linear_roots, vector<pair<ZZ_p, ZZ_p>>& points);
void evaluate_polynomial_points(vector<pair<ZZ_p, ZZ_p>>& points, const ZZ_pX& poly, int start, int length);
void generate_random_BIG(BIG& random);
std::vector<uint8_t> serialize_ECP(const ECP& point);
ECP deserialize_ECP(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> serialize_ZZ_pX(const ZZ_pX& poly);
ZZ_pX deserialize_ZZ_pX(const std::vector<uint8_t>& bytes);

#endif

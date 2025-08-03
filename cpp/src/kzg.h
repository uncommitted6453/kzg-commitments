#ifndef KZG_H
#define KZG_H

#include <vector>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>
#include <NTL/ZZX.h>
#include <core.h>

using namespace std;
using namespace BN158;
using namespace NTL;
using namespace core;

namespace kzg {

struct public_params {
  std::vector<ECP> _G1;
  std::vector<ECP2> _G2;
};

public_params trusted_setup(int num_coeff);

public_params load_params_file(const std::string& filename);

ECP polyeval_G1(public_params& setup, const ZZ_pX& P);
ECP2 polyeval_G2(public_params& setup, const ZZ_pX& P);

ECP commit(public_params& setup, const ZZ_pX& P);
ECP create_proof(public_params& setup, const ZZ_pX& P, int offset, int length);
bool verify(public_params& setup, ECP& commit, ECP& proof, std::vector<pair<ZZ_p, ZZ_p>>& points);

bool export_params_file(public_params& setup, const std::string& filename = "kzg_public");

}

#endif

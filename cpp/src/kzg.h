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

class commit {
private:
  ECP curve_point;

public:
  commit(ECP _curve_point) : curve_point(_curve_point) {}
  ECP& get_curve_point() { return curve_point; }
};

class proof {
private:
  ECP curve_point;

public:
  proof(ECP _curve_point) : curve_point(_curve_point) {}
  ECP& get_curve_point() { return curve_point; }
};

class trusted_setup {
private:
  std::vector<ECP> _G1;
  std::vector<ECP2> _G2;
  
  ECP polyeval_G1(const ZZ_pX& P);
  ECP2 polyeval_G2(const ZZ_pX& P);

public:
  trusted_setup(int num_coeff);
  trusted_setup(const std::string& filename);
  
  commit create_commit(const ZZ_pX& P);  
  
  proof create_proof(const ZZ_pX &P, int offset, int length);
  bool verify_proof(commit& commit, proof& proof, std::vector<pair<ZZ_p, ZZ_p>>& points);
  
  void export_setup(const std::string& filename = "kzg_public");
};

}

#endif

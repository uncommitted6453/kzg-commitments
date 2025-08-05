#ifndef KZG_H
#define KZG_H

#include <vector>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>
#include <big_B160_56.h>
#include <NTL/ZZX.h>
#include <core.h>

#define MAX_CHUNK_SIZE 19

using namespace std;
using namespace BN158;
using namespace B160_56;
using namespace NTL;
using namespace core;

namespace kzg {

class blob {
private:
  vector<pair<ZZ_p, ZZ_p>> data;

public:
  static blob from_string(string s);
  static blob from_string(string s, int offset);
  static blob from_bytes(const uint8_t* bytes, int byte_offset, int byte_length, int chunk_size);
  
  blob(vector<pair<ZZ_p, ZZ_p>>& _data) : data(_data) {}
  vector<pair<ZZ_p, ZZ_p>>& get_data() { return data; }
};

class poly {
private:
  ZZ_pX data;

public:
  static poly from_blob(blob blob);
  static poly from_random(int num_coeff);
  
  poly(ZZ_pX _data) : data(_data) {}
  const ZZ_pX& get_poly() const { return data; }
};

class commit {
private:
  ECP curve_point;

public:
  commit(ECP _curve_point) : curve_point(_curve_point) {}
  ECP& get_curve_point() { return curve_point; }

  std::vector<uint8_t> serialize();
  static commit deserialize(const std::vector<uint8_t>&);
};

class proof {
private:
  ECP curve_point;

public:
  proof(ECP _curve_point) : curve_point(_curve_point) {}
  ECP& get_curve_point() { return curve_point; }

  std::vector<uint8_t> serialize();
  static proof deserialize(const std::vector<uint8_t>&);
};

class trusted_setup {
private:
  std::vector<ECP> _G1;
  std::vector<ECP2> _G2;
  
  ECP polyeval_G1(const ZZ_pX& P);
  ECP2 polyeval_G2(const ZZ_pX& P);

  void generate_elements_range(int start, int end, BIG s_i, BIG s);
public:
  trusted_setup(int num_coeff);
  trusted_setup(const std::string& filename);
  
  commit create_commit(const kzg::poly& poly);
  bool verify_commit(kzg::commit& commit, const kzg::poly& poly);
  
  proof create_proof(const kzg::poly& poly, int byte_offset, int byte_length, int chunk_size);
  proof create_proof(const kzg::poly& poly, int chunk_offset, int chunk_length);
  bool verify_proof(commit& commit, proof& proof, blob& expected_data);
  
  void export_setup(const std::string& filename = "kzg_public");
};

}

#endif

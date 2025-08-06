#ifndef KZG_H
#define KZG_H

#include <kzg_config.h>

#include <vector>
#include <NTL/ZZX.h>

using namespace std;
using namespace NTL;

namespace kzg {

extern int CURVE_ORDER_BYTES;

extern void init();

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
  
  std::vector<uint8_t> serialize();
  static poly deserialize(const std::vector<uint8_t>&);
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

  void generate_elements_range(int start, int end, const std::vector<BIG>& s_powers);

public:
  /**
  * @brief Performs the trusted setup step of the KZG commitment scheme.
  * 
  * Computes the group elements G1[s^i] and G2[s^i], up to num_coeff. The 
  * parameter num_coeff determines the maximum degree of polynomials 
  * that can be committed with the setup. 
  * 
  * @param num_coeff The number of group elements to generate
  */
  trusted_setup(int num_coeff);
  
  /**
  * @brief Constructs a trusted setup from a file exported with kzg::trusted_setup::export_setup.
  * 
  * @param filename Path to the binary file containing the trusted setup data
  */
  trusted_setup(const std::string& filename);
  
  /**
  * @brief Creates a KZG commitment for a given polynomial
  * 
  * Computes the commitment C = [P(s)]₁ where P is the polynomial and s is the
  * secret value from the trusted setup. The commitment is a point on the G1 curve.
  * 
  * @param poly The polynomial to create a commitment for
  * @return A KZG commitment object containing the G1 curve point
  */
  commit create_commit(const kzg::poly& poly);
  
  /**
  * @brief Verifies that a commitment matches a given polynomial
  * 
  * Checks if the provided commitment is the correct KZG commitment for the given
  * polynomial by recomputing the commitment and comparing curve points.
  * 
  * @param commit The commitment to verify
  * @param poly The polynomial that should correspond to the commitment
  * @return true if the commitment is valid for the polynomial, false otherwise
  */
  bool verify_commit(kzg::commit& commit, const kzg::poly& poly);
  
  /**
  * @brief Creates a KZG proof for a specific byte range of data
  * 
  * Generates a proof that the polynomial encodes the specified byte range, where the
  * polynomial was generated such that each point represents chunk_size bytes.
  * 
  * @param poly The polynomial to create a proof for
  * @param byte_offset Starting byte position (must be multiple of chunk_size)
  * @param byte_length Number of bytes to prove (must be multiple of chunk_size)
  * @param chunk_size Size of each chunk in bytes (must be < CURVE_ORDER_BYTES)
  * @return A KZG proof object
  * @throws invalid_argument if parameters don't meet the required constraints
  */
  proof create_proof(const kzg::poly& poly, int byte_offset, int byte_length, int chunk_size);


  /**
  * @brief Creates a KZG proof for a specific chunk range
  * 
  * Generates a proof that the polynomial correctly encodes the specified chunk range
  * where the polynomial was generated such that each point represents chunk_size bytes.
  * 
  * @param poly The polynomial to create a proof for
  * @param chunk_offset Starting chunk position
  * @param chunk_length Number of chunks to prove
  * @return A KZG proof object
  */
  proof create_proof(const kzg::poly& poly, int chunk_offset, int chunk_length);
  
  /**
  * @brief Verifies a KZG proof against a commitment and expected data
  * 
  * Verifies that the proof correctly demonstrates that the committed polynomial
  * evaluates to the expected values at the specified points.
  * 
  * @param commit The commitment to verify against
  * @param proof The proof to verify
  * @param expected_data The blob containing expected evaluation points and values
  * @return true if the proof is valid, false otherwise
  */
  bool verify_proof(commit& commit, proof& proof, blob& expected_data);
  
  /**
  * @brief Exports the trusted setup to a binary file
  * 
  * Serializes the trusted setup (G1 and G2 group elements) to a binary file
  * for later reuse.
  * 
  * @param filename Path where the trusted setup should be exported (default: "kzg_public")
  */
  void export_setup(const std::string& filename = "kzg_public");
};

}

#endif

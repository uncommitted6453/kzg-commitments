#ifndef KZG_H
#define KZG_H

#include <kzg_config.h>

#include <vector>
#include <NTL/ZZX.h>

using namespace std;
using namespace NTL;

/**
 * @namespace kzg
 * @brief KZG polynomial commitment scheme functions
 * 
 * This namespace contains all classes and functions necessary for producing and verifying
 * KZG commitments. 
 * 
 * The main workflow involves:
 * 1. Initializing the library with kzg::init()
 * 2. Creating a trusted setup using kzg::trusted_setup
 * 3. Converting your data to a blob using kzg::blob
 * 4. Generating a polynomial from the blob using kzg::poly
 * 5. Creating commitments and proofs using the trusted setup
 * 6. Verifying the commitments and proofs
 */
namespace kzg {

extern int CURVE_ORDER_BYTES;

#define MAX_CHUNK_BYTES (kzg::CURVE_ORDER_BYTES - 1)

/**
* @brief Initialize the library
*
* This function must be called prior to any other library function.
*/
void init();

class blob {
private:
  vector<pair<ZZ_p, ZZ_p>> data;

public:
  blob(vector<pair<ZZ_p, ZZ_p>>& _data) : data(_data) {}
  vector<pair<ZZ_p, ZZ_p>>& get_data() { return data; }

  /**
  * @brief Generate a vector of evaluation points encoding a string
  *
  * Use this function along with poly::from_blob to obtain a polynomial 
  * encoding your string that can be committed to. 
  *
  * @param s The string to construct a blob from
  * @return A blob object containing the vector of evaluation points
  */
  static blob from_string(string s);

  /**
  * @brief Generate a vector of evaluation points encoding a string
  *
  * Use this function along with poly::from_blob to obtain a polynomial 
  * encoding your string that can be committed to. 
  *
  * @param s The string to construct a blob from
  * @param offset The offset the evaluation points should have (useful for 
                  committing or verifying a specific section of data)
  * @return A blob object containing the vector of evaluation points
  */
  static blob from_string(string s, int offset);
 
  /**
  * @brief Generate a vector of evaluation points encoding a buffer of bytes
  *
  * Each point represents chunk_size bytes. Use this function along with 
  * poly::from_blob to obtain a polynomial encoding your data that can be
  * committed to or used in verification. 
  *
  * @param bytes The byte buffer that you would like to encode
  * @param byte_offset The offset into the buffer to begin encoding (must be multiple of chunk_size)
  * @param byte_length The number of bytes that should be encoded from the offset (must be multiple of chunk_size)
  * @param chunk_size The number of bytes that each point represents (must be at most MAX_CHUNK_BYTES)
  * @return A blob object containing the vector of evaluation points
  * @throws invalid_argument if parameters don't meet the required constraints
  */
  static blob from_bytes(const uint8_t* bytes, int byte_offset, int byte_length, int chunk_size);
};

class poly {
private:
  ZZ_pX data;
  
public:
  poly(ZZ_pX _data) : data(_data) {}
  const ZZ_pX& get_poly() const { return data; }
  /**
  * @brief Constructs a polynomial fitting the evaluation points in a blob
  * 
  * A standard workflow would be to generate a blob from the data you would like to commit or verify
  * and then use this function to obtain a polynomial that can be used with KZG. 
  * 
  * @param blob The evaluation points that you want to generate a polynomial for
  * @return The polynomial fitted to the evaluation points
  */
  static poly from_blob(blob blob);

  /**
  * @brief Serialize the polynomial into bytes
  * 
  * Use of this method is recommended for transmission or storage of the polynomial.
  * 
  * @return A vector of bytes representing the polynomial
  */
  std::vector<uint8_t> serialize();
  
  /**
  * @brief Deserialize the polynomial from bytes
  * 
  * Use of this method is recommended for transmission or storage of the polynomial.
  * 
  * @param bytes The vector of bytes containing the serialized polynomial object
  * @return The deserialized polynomial object
  */
  static poly deserialize(const std::vector<uint8_t>&);
};

class commit {
private:
  ECP curve_point;

public:
  commit(ECP _curve_point) : curve_point(_curve_point) {}
  ECP& get_curve_point() { return curve_point; }
  /**
  * @brief Serialize the commit (a point on the elliptic curve) into bytes
  * 
  * Use of this method is recommended for transmission or storage of the proof.
  * 
  * @return A vector of bytes representing the commit
  */
  std::vector<uint8_t> serialize();
  
  /**
  * @brief Deserialize the commit (a point on the elliptic curve) from bytes
  * 
  * Use of this method is recommended for transmission or storage of the commit.
  * 
  * @param bytes The vector of bytes containing the serialized commit object
  * @return The deserialized commit object
  */
  static commit deserialize(const std::vector<uint8_t>&);
};

class proof {
private:
  ECP curve_point;

public:
  proof(ECP _curve_point) : curve_point(_curve_point) {}
  ECP& get_curve_point() { return curve_point; }

  /**
  * @brief Serialize the proof (a point on the elliptic curve) into bytes
  * 
  * Use of this method is recommended for transmission or storage of the proof.
  * 
  * @return A vector of bytes representing the proof
  */
  std::vector<uint8_t> serialize();

  /**
  * @brief Deserialize the proof (a point on the elliptic curve) from bytes
  * 
  * Use of this method is recommended for transmission or storage of the proof.
  * 
  * @param bytes The vector of bytes containing the serialized proof object
  * @return The deserialized proof object
  */
  static proof deserialize(const std::vector<uint8_t>& bytes);
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
  * @brief Performs the trusted setup step of the KZG commitment scheme
  * 
  * Computes the group elements G1[s^i] and G2[s^i], up to num_coeff. The
  * maximum degree of a polynomial that can be committed with this setup
  * is num_coeff - 1.
  * 
  * @param num_coeff The number of group elements to generate (num_coeff > 1)
  * @throws invalid_argument if the constraint isn't met
  */
  trusted_setup(int num_coeff);
  
  /**
  * @brief Loads a trusted setup from a file exported with kzg::trusted_setup::export_setup
  * 
  * @param filename Path to the binary file containing the trusted setup data
  * @throws runtime_error for a inaccessible / bad file
  */
  trusted_setup(const std::string& filename);
  
  /**
  * @brief Creates a KZG commitment for a given polynomial
  * 
  * Computes the commitment C = [P(s)]₁ where P is the polynomial and s is the
  * secret value from the trusted setup. The commitment is a point on the G1 curve.
  * 
  * @param poly The polynomial to create a commitment for (polynomial degree be at most one less than the setup size)
  * @return A KZG commitment object containing the G1 curve point
  * @throws invalid_argument if constraint not met
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
  * @param chunk_size Size of each chunk in bytes (must be at most MAX_CHUNK_BYTES)
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
  * @throws invalid_argument if chunk_length < 1
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
  * @throws invalid_argument if expected_data is empty
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

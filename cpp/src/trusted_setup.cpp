#include <kzg.h>

#include <fstream>
#include <cstdint>
#include <thread>
#include <vector>
#include <functional>
#include "util.h"

int kzg::CURVE_ORDER_BYTES;

static constexpr size_t G1_OCTET_SIZE = 2 * MODBYTES_CURVE + 1;
static constexpr size_t G2_OCTET_SIZE = 4 * MODBYTES_CURVE + 1;

void kzg::init() {
  ZZ ZZ_curve_order = ZZ_from_BIG(CURVE_Order);
  ZZ_p::init(ZZ_curve_order);
  kzg::CURVE_ORDER_BYTES = NumBytes(ZZ_curve_order);
}

/**
 * @brief Performs the trusted setup step of the KZG commitment scheme.
 * 
 * Computes the group elements G1[s^i] and G2[s^i], up to num_coeff. The 
 * parameter num_coeff also determines the maximum degree of polynomials 
 * that can be committed with the setup. 
 * 
 * @param num_coeff The number of group elements to generate
 */
kzg::trusted_setup::trusted_setup(int num_coeff) {
  BIG BIG_s;
  generate_random_BIG(BIG_s);
  ZZ_p s = conv<ZZ_p>(ZZ_from_BIG(BIG_s));

  _G1.resize(num_coeff);
  _G2.resize(num_coeff);

  std::vector<BIG> s_powers(num_coeff);
  for (int i = 0; i < num_coeff; i++) {
    ZZ_p s_i = power(s, i);
    BIG_from_ZZ(s_powers[i], rep(s_i));
  }

  unsigned int num_threads = std::thread::hardware_concurrency();
  
  if (num_threads == 0) {
    num_threads = 4;
  } else if (num_coeff < num_threads) {
    generate_elements_range(0, num_coeff, std::cref(s_powers));
  } else {
    int elements_per_thread = num_coeff / num_threads;
    int remaining_elements = num_coeff % num_threads;
    
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    int start = 0;
    for (unsigned int t = 0; t < num_threads; t++) {
      // distribute remaining elements 1 each to first remaining_elements threads
      int num_elements_to_handle = elements_per_thread;
      if (t < remaining_elements)
        num_elements_to_handle += 1;

      int end = start + num_elements_to_handle;
      if (start < num_coeff) {
        threads.push_back(std::thread(
          &kzg::trusted_setup::generate_elements_range, 
          this, start, end, std::cref(s_powers)
        ));
      }
      start = end;
    }
    
    for (auto& thread : threads) {
      thread.join();
    }
  }
}

/**
 * @brief Constructs a trusted setup from a file exported with kzg::trusted_setup::export_setup.
 * 
 * @param filename Path to the binary file containing the trusted setup data
 */
kzg::trusted_setup::trusted_setup(const std::string& filename) {
  ZZ z = ZZ_from_BIG(CURVE_Order);
  ZZ_p::init(z);
  
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "failed to import" << std::endl;
    return;
  }
  
  uint64_t num_coeffs;
  file.read(reinterpret_cast<char*>(&num_coeffs), sizeof(num_coeffs));
  
  _G1.reserve(static_cast<size_t>(num_coeffs));
  _G2.reserve(static_cast<size_t>(num_coeffs));
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    uint32_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    
    char buffer[G1_OCTET_SIZE];
    file.read(buffer, len);
    
    octet oct = {static_cast<int>(len), G1_OCTET_SIZE, buffer};
    ECP point;
    if (ECP_fromOctet(&point, &oct)) {
      _G1.push_back(point);
    } else {
      std::cerr << "point at index " << i << " is invalid" << std::endl;
    }
  }
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    uint32_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    
    char buffer[G2_OCTET_SIZE];
    file.read(buffer, len);
    
    octet oct = {static_cast<int>(len), G2_OCTET_SIZE, buffer};
    ECP2 point;
    if (ECP2_fromOctet(&point, &oct)) {
      _G2.push_back(point);
    } else {
      std::cerr << "point at index " << i << " is invalid" << std::endl;
    }
  }
  
  file.close();
  std::cout << "loaded group elements from " << filename << " with num_coeffs=" << num_coeffs << "" << std::endl;
}

void kzg::trusted_setup::generate_elements_range(int start, int end, const std::vector<BIG>& s_powers) {
  for (int i = start; i < end; i++) {
    ECP G1_s_i;
    ECP_generator(&G1_s_i);
    PAIR_G1mul(&G1_s_i, const_cast<BIG&>(s_powers[i]));
    _G1[i] = G1_s_i;

    ECP2 G2_s_i;
    ECP2_generator(&G2_s_i);
    PAIR_G2mul(&G2_s_i, const_cast<BIG&>(s_powers[i]));
    _G2[i] = G2_s_i;
  }
}


/**
 * @brief Creates a KZG commitment for a given polynomial
 * 
 * Computes the commitment C = [P(s)]₁ where P is the polynomial and s is the
 * secret value from the trusted setup. The commitment is a point on the G1 curve.
 * 
 * @param poly The polynomial to create a commitment for
 * @return A KZG commitment object containing the G1 curve point
 */
kzg::commit kzg::trusted_setup::create_commit(const kzg::poly& poly) {
  return kzg::commit(polyeval_G1(poly.get_poly()));
}

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
bool kzg::trusted_setup::verify_commit(kzg::commit& commit, const kzg::poly& poly) {
  kzg::commit expected_commit = create_commit(poly);
  return ECP_equals(&commit.get_curve_point(), &expected_commit.get_curve_point());
}

ECP kzg::trusted_setup::polyeval_G1(const ZZ_pX& P) {
  if (deg(P) == -1) {
    ECP inf;
    ECP_inf(&inf);
    return inf;
  }

  BIG coeff_i;
  BIG_from_ZZ(coeff_i, rep(P[0]));
  
  ECP res;
  ECP_copy(&res, &_G1[0]);
  PAIR_G1mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    BIG_from_ZZ(coeff_i, rep(P[i]));
    
    ECP term;
    ECP_copy(&term, &_G1[i]);
    PAIR_G1mul(&term, coeff_i);
    
    ECP_add(&res, &term);
  }
  
  return res;
}

ECP2 kzg::trusted_setup::polyeval_G2(const ZZ_pX& P) {
  if (deg(P) == -1) {
    ECP2 inf;
    ECP2_inf(&inf);
    return inf;
  }

  BIG coeff_i;
  BIG_from_ZZ(coeff_i, rep(P[0]));
  
  ECP2 res;
  ECP2_copy(&res, &_G2[0]);
  PAIR_G2mul(&res, coeff_i);
  
  for (int i = 1; i <= deg(P); i++) {
    BIG_from_ZZ(coeff_i, rep(P[i]));
    
    ECP2 term;
    ECP2_copy(&term, &_G2[i]);
    PAIR_G2mul(&term, coeff_i);
    
    ECP2_add(&res, &term);
  }
  
  return res;
}

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
kzg::proof kzg::trusted_setup::create_proof(const kzg::poly& poly, int byte_offset, int byte_length, int chunk_size) {
  if (chunk_size > CURVE_ORDER_BYTES - 1)
    throw invalid_argument("chunk_size must be lower than CURVE_ORDER_BYTES.");
  else if (byte_offset % chunk_size != 0)
    throw invalid_argument("byte_offset is not a multiple of chunk_size.");
  else if (byte_length % chunk_size != 0)
    throw invalid_argument("byte_length is not a multiple of chun_size.");
  
  return create_proof(poly, byte_offset / chunk_size, byte_length / chunk_size);
}

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
kzg::proof kzg::trusted_setup::create_proof(const kzg::poly& poly, int chunk_offset, int chunk_length) {
  const ZZ_pX& P = poly.get_poly();
  
  vector<pair<ZZ_p, ZZ_p>> points;
  evaluate_polynomial_points(points, P, chunk_offset, chunk_length);
  
  ZZ_pX I, Z;
  linear_roots_and_polyfit(I, Z, points);
  ZZ_pX q = (P - I) / Z;
  
  return kzg::proof(polyeval_G1(q));
}

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
bool kzg::trusted_setup::verify_proof(kzg::commit& commit, kzg::proof& proof, kzg::blob& expected_data) {
  vector<pair<ZZ_p, ZZ_p>>& points = expected_data.get_data();
  
  ZZ_pX I, Z;
  linear_roots_and_polyfit(I, Z, points);
  
  ECP2 p1 = polyeval_G2(Z);
  FP12 v1;
  PAIR_ate(&v1, &p1, &proof.get_curve_point());
  PAIR_fexp(&v1);
  
  ECP p2 = polyeval_G1(I);
  ECP_neg(&p2);
  ECP_add(&p2, &commit.get_curve_point());
  FP12 v2;
  PAIR_ate(&v2, &_G2[0], &p2);
  PAIR_fexp(&v2);
  
  return FP12_equals(&v1, &v2);
}

/**
 * @brief Exports the trusted setup to a binary file
 * 
 * Serializes the trusted setup (G1 and G2 group elements) to a binary file
 * for later reuse.
 * 
 * @param filename Path where the trusted setup should be exported (default: "kzg_public")
 */
void kzg::trusted_setup::export_setup(const std::string& filename) {
  std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << "failed to export" << std::endl;
    return;
  }
  
  uint64_t num_coeffs = static_cast<uint64_t>(_G1.size());
  file.write(reinterpret_cast<const char*>(&num_coeffs), sizeof(num_coeffs));
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    char buffer[G1_OCTET_SIZE];
    octet oct = {0, G1_OCTET_SIZE, buffer};
    ECP_toOctet(&oct, &_G1[i], false);
    // std::cout << oct.len << std::endl;
    
    uint32_t len = static_cast<uint32_t>(oct.len);
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(oct.val, oct.len);
  }
  
  for (uint64_t i = 0; i < num_coeffs; i++) {
    char buffer[G2_OCTET_SIZE];
    octet oct = {0, G2_OCTET_SIZE, buffer};
    ECP2_toOctet(&oct, &_G2[i], false);

    uint32_t len = static_cast<uint32_t>(oct.len);
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(oct.val, oct.len);
  }
  
  file.close();
  std::cout << "exported group elements to " << filename << " with num_coeffs=" << num_coeffs << "" << std::endl;
}


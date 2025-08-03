#ifndef KZG_H
#define KZG_H

#include <vector>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>
#include <NTL/ZZX.h>
#include <core.h>
#include <cstdint>
#include <string>
#include <utility>

using KZGPolynomial = std::vector<uint8_t>;
using KZGCommitment = std::vector<uint8_t>;
using KZGProof = std::vector<uint8_t>;
using KZGFieldElement = std::vector<uint8_t>;
using KZGEvaluationPoint = std::pair<KZGFieldElement, KZGFieldElement>; 

using namespace std;
using namespace BN158;
using namespace NTL;
using namespace core;

class KZG {
private:
  std::vector<ECP> _G1;
  std::vector<ECP2> _G2;
  
  ECP polyeval_G1(const ZZ_pX& P);
  ECP2 polyeval_G2(const ZZ_pX& P);

public:
  KZG(int num_coeff);
  KZG(const std::string& filename);
  KZGCommitment commit(const KZGPolynomial& polynomial);
  KZGProof create_proof(const KZGPolynomial& polynomial, int offset, int length);
  bool verify(const KZGCommitment& commitment, const KZGProof& proof, 
              const std::vector<KZGEvaluationPoint>& points);
  
  static KZGPolynomial create_polynomial_from_string(const std::string& data, int offset = 0);
  static std::vector<KZGEvaluationPoint> create_evaluation_points_from_string(const std::string& data, int offset = 0);
  
  void export_setup(const std::string& filename = "kzg_public");
};

#endif

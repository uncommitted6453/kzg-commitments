#include <stdio.h>
#include <ecp_BN158.h>
#include <ecp2_BN158.h>
#include <fp12_BN158.h>
#include <pair_BN158.h>
#include <big_160_56.h>
#include "poly.h"

void kzg_trusted_setup(ECP_BN158 G1_vars[], ECP2_BN158 G2_vars[], int num_vars) {
  FP_BN158 secret;
  FP_BN158_from_int(&secret, 42);
  
  BIG_160_56 BIG_i;
  BIG_160_56_zero(BIG_i);
  
  for (int i = 0; i < num_vars; i++) {
    FP_BN158 s_i;
    BIG_160_56 BIG_s_i;
    
    FP_BN158_pow(&s_i, &secret, BIG_i);
    FP_BN158_redc(BIG_s_i, &s_i);
    
    ECP_BN158_generator(&G1_vars[i]);
    ECP_BN158_mul(&G1_vars[i], BIG_s_i);
    
    ECP2_BN158_generator(&G2_vars[i]);
    ECP2_BN158_mul(&G2_vars[i], BIG_s_i);
    
    BIG_160_56_inc(BIG_i, 1);
  }
}

void kzg_evaluate_polynomial_G1(ECP_BN158 *res, ECP_BN158 G1_vars[], POLY *P) {
  ECP_BN158 term;
  BIG_160_56 coeff_i;
  
  FP_BN158_redc(coeff_i, &P->coeff[0]);
  ECP_BN158_copy(res, &G1_vars[0]);
  ECP_BN158_mul(res, coeff_i);
  
  for (int i = 1; i < P->num_coeff; i++) {
    FP_BN158_redc(coeff_i, &P->coeff[i]);
    
    ECP_BN158_copy(&term, &G1_vars[i]);
    ECP_BN158_mul(&term, coeff_i);
    
    ECP_BN158_add(res, &term);
  }
}

void kzg_evaluate_polynomial_G2(ECP2_BN158 *res, ECP2_BN158 G2_vars[], POLY *P) {
  ECP2_BN158 term;
  BIG_160_56 coeff_i;
  
  FP_BN158_redc(coeff_i, &P->coeff[0]);
  ECP2_BN158_copy(res, &G2_vars[0]);
  ECP2_BN158_mul(res, coeff_i);
  
  for (int i = 1; i < P->num_coeff; i++) {
    FP_BN158_redc(coeff_i, &P->coeff[i]);
    
    ECP2_BN158_copy(&term, &G2_vars[i]);
    ECP2_BN158_mul(&term, coeff_i);
    
    ECP2_BN158_add(res, &term);
  }
}

void kzg_create_single_proof(ECP_BN158 *single_proof, ECP_BN158 G1_vars[], POLY *P, int z, int y) {
  // Take the point (z, y=P(z))
  FP_BN158 fp_y;
  FP_BN158_from_int(&fp_y, y);
  
  // Compute I(X) = (P(X) - y) / (x - z);
  POLY *Z = POLY_new();
  POLY *I = POLY_new();
  POLY *r = POLY_new();
  
  POLY_copy(r, P);
  FP_BN158_sub(&r->coeff[0], &r->coeff[0], &fp_y);
  POLY_from_x_sub_a(Z, z);
  POLY_mod_div(r, Z, I);
  
  // Compute an evaluation of the proof polynomial I(s)
  kzg_evaluate_polynomial_G1(single_proof, G1_vars, I);
  
  POLY_kill(Z);
  POLY_kill(I);
  POLY_kill(r);
}

void kzg_verify_single_proof(
  ECP_BN158 *poly_commit,
  ECP_BN158 *single_proof,
  ECP_BN158 G1_vars[],
  ECP2_BN158 G2_vars[],
  int z, int y
) {
  // Take the point (z, y=P(z))
  FP_BN158 fp_z;
  FP_BN158_from_int(&fp_z, z);
  BIG_160_56 big_z;
  FP_BN158_redc(big_z, &fp_z);
  
  FP_BN158 fp_y;
  FP_BN158_from_int(&fp_y, y);
  BIG_160_56 big_y;
  FP_BN158_redc(big_y, &fp_y);
  
  // Left Hand Side
  ECP2_BN158 p1;
  ECP2_BN158_copy(&p1, &G2_vars[0]);
  ECP2_BN158_mul(&p1, big_z);
  ECP2_BN158_neg(&p1);
  ECP2_BN158_add(&p1, &G2_vars[1]);
  
  FP12_BN158 v1;
  PAIR_BN158_ate(&v1, &p1, single_proof);
  FP12_BN158_output(&v1); printf("\n");
  
  // Right Hand Side
  ECP_BN158 p2;
  ECP_BN158_copy(&p2, &G1_vars[0]);
  ECP_BN158_mul(&p2, big_y);
  ECP_BN158_neg(&p2);
  ECP_BN158_add(&p2, poly_commit);
  
  FP12_BN158 v2;
  PAIR_BN158_ate(&v2, &G2_vars[0], &p2);
  FP12_BN158_output(&v2); printf("\n");
}

int main() {
  /* --------------------- KZG Setup ---------------- */
  
  ECP_BN158 G1_vars[32];
  ECP2_BN158 G2_vars[32];
  kzg_trusted_setup(G1_vars, G2_vars, 32);
  
  /* ------------ Create and Commit Polynomial ---------- */
  
  POLY *A = POLY_new();
  POLY *B = POLY_new();
  
  // P(x) = (x - 3) * (x - 4) * (x - 10) * (x - 12)
  POLY_from_x_sub_a(A, 3);
  POLY_from_x_sub_a(B, 4);
  POLY_mul(A, B);
  POLY_from_x_sub_a(B, -10);
  POLY_mul(A, B);
  POLY_from_x_sub_a(B, -12);
  POLY_mul(A, B);
  
  // Take an evaluation of the polynomial in G1, P(s)
  ECP_BN158 poly_commit;
  kzg_evaluate_polynomial_G1(&poly_commit, G1_vars, A);
  
  /* ----------- Create single proof --------------- */
  
  ECP_BN158 single_proof;
  kzg_create_single_proof(&single_proof, G1_vars, A, 20, 529920);
  ECP_BN158_output(&single_proof);
  
  /* ------------- Verify the proof ------------ */
  
  kzg_verify_single_proof(&poly_commit, &single_proof, G1_vars, G2_vars, 20, 529920);

  /* -------------- Cleanup -------------------- */
  
  POLY_kill(A);
  POLY_kill(B);
  
  return 0;
}

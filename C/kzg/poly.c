#include "poly.h"
#include <stdlib.h>
#include <string.h>

POLY *POLY_new() {
  POLY *P = malloc(sizeof(POLY));
  P->coeff = NULL;
  P->num_coeff = 0;
  return P;
}

void POLY_realloc(POLY *P, int num_coeff) {
  if (!P->coeff)
    P->coeff = malloc(num_coeff * sizeof(FP_BN158));
  else
    P->coeff = realloc(P->coeff, num_coeff * sizeof(FP_BN158));
  
  for (int i = 0; i < num_coeff; i++)
    FP_BN158_zero(&P->coeff[i]);
  
  P->num_coeff = num_coeff;
}

void POLY_from_x_sub_a(POLY *P, int a) {
  POLY_realloc(P, 2);
  FP_BN158_from_int(&P->coeff[0], -a);
  FP_BN158_from_int(&P->coeff[1], 1);
}

void POLY_copy(POLY *P, POLY *Q) {
  POLY_realloc(P, Q->num_coeff);
  P->num_coeff = Q->num_coeff;
  memcpy(P->coeff, Q->coeff, P->num_coeff * sizeof(FP_BN158));
}

void POLY_sort(POLY **P, POLY **Q) {
  if ((*Q)->num_coeff > (*P)->num_coeff) {
    POLY* tmp = *P;
    *P = *Q;
    *Q = tmp;
  }
}

void POLY_eval(FP_BN158 *res, POLY *P, FP_BN158 *x) {
  FP_BN158_zero(res);
  
  BIG_160_56 exp;
  BIG_160_56_zero(exp);
  
  for (int i = 0; i < P->num_coeff; i++) {
    FP_BN158 term;
    FP_BN158_pow(&term, x, exp);
    FP_BN158_mul(&term, &term, &P->coeff[i]);
    
    FP_BN158_add(res, res, &term);
    
    BIG_160_56_inc(exp, 1);
  }
}

void POLY_mul(POLY *P, POLY *Q) {
  POLY *out = P;
  POLY_sort(&P, &Q);
  
  POLY *res = POLY_new();
  POLY_realloc(res, P->num_coeff + Q->num_coeff - 1);
  
  for (int i = 0; i < P->num_coeff; i++) {
    for (int j = 0; j < Q->num_coeff; j++) {
      FP_BN158 digit;
      FP_BN158_mul(&digit, &P->coeff[i], &Q->coeff[j]);
      FP_BN158_add(&res->coeff[i + j], &res->coeff[i + j], &digit);
    }
  }
  
  POLY_copy(out, res);
  POLY_kill(res);
}

void POLY_mod_div(POLY *R, POLY *D, POLY *Q) {
  POLY_realloc(Q, R->num_coeff - D->num_coeff + 1);
  
  int i = R->num_coeff - 1;
  while (i >= D->num_coeff - 1) {
    FP_BN158 t;
    FP_BN158_inv(&t, &D->coeff[D->num_coeff - 1], NULL);
    FP_BN158_mul(&t, &t, &R->coeff[i]);
    
    FP_BN158_copy(&Q->coeff[i - D->num_coeff + 1], &t);
    
    for (int j = 0; j < D->num_coeff; j++) {
      FP_BN158 digit;
      FP_BN158_mul(&digit, &D->coeff[D->num_coeff - 1 - j], &t);
      FP_BN158_sub(&R->coeff[i - j], &R->coeff[i - j], &digit);
    }
    
    while (i >= D->num_coeff - 1 && FP_BN158_iszilch(&R->coeff[i])) i--;
  }
}

void POLY_add(POLY *P, POLY *Q) {
  POLY *out = P;
  POLY_sort(&P, &Q);
  
  POLY *res = POLY_new();
  POLY_realloc(res, P->num_coeff);
  
  for (int i = 0; i < Q->num_coeff; i++)
    FP_BN158_add(&res->coeff[i], &P->coeff[i], &Q->coeff[i]);
  
  for (int i = Q->num_coeff; i < P->num_coeff; i++)
    res->coeff[i] = Q->coeff[i];
  
  POLY_copy(out, res);
  POLY_kill(res);
}

void POLY_sub(POLY *P, POLY *Q) {
  POLY *out = P;
  POLY_sort(&P, &Q);
  
  POLY *res = POLY_new();
  POLY_realloc(res, P->num_coeff);
  
  for (int i = 0; i < Q->num_coeff; i++)
    FP_BN158_sub(&res->coeff[i], &P->coeff[i], &Q->coeff[i]);
  
  for (int i = Q->num_coeff; i < P->num_coeff; i++)
    res->coeff[i] = Q->coeff[i];
  
  POLY_copy(out, res);
  POLY_kill(res);
}

void POLY_kill(POLY *P) {
  if (P->coeff)
    free(P->coeff);
  free(P);
}

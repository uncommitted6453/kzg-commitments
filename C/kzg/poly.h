#ifndef POLY_H
#define POLY_H

#include <fp_BN158.h>

typedef struct {
  FP_BN158 *coeff;
  int num_coeff;
} POLY;

extern POLY *POLY_new();
extern void POLY_from_x_sub_a(POLY *P, int a);
extern void POLY_add(POLY *P, POLY *Q);
extern void POLY_sub(POLY *P, POLY *Q);
extern void POLY_mul(POLY *P, POLY *Q);
extern void POLY_mod_div(POLY *R, POLY *D, POLY *Q);
extern void POLY_eval(FP_BN158 *res, POLY *P, FP_BN158 *x);
extern void POLY_copy(POLY *P, POLY *Q);
extern void POLY_kill(POLY *P);

#endif

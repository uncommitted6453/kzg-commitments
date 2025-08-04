#include "kzg.h"
#include "util.h"

kzg::poly kzg::poly::from_blob(kzg::blob blob) {
  return kzg::poly(polyfit(blob.get_data()));
}

kzg::poly kzg::poly::from_random(int num_coeff) {
  ZZ_pX p;
  
  for (int i = 0; i < num_coeff; i++) {
    ZZ_p a;
    a = rand();
    a = power(a, rand());
    SetCoeff(p, i, a);
  }
  
  return kzg::poly(p);
}

#include "kzg.h"
#include "util.h"

kzg::poly kzg::poly::from_blob(kzg::blob blob) {
  return kzg::poly(polyfit(blob.get_data()));
}

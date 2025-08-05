#include "kzg.h"
#include "util.h"

kzg::poly kzg::poly::from_blob(kzg::blob blob) {
  return kzg::poly(polyfit(blob.get_data()));
}

std::vector<uint8_t> kzg::poly::serialize() {
  return serialize_ZZ_pX(data);
}

kzg::poly kzg::poly::deserialize(const std::vector<uint8_t>& bytes) {
  ZZ_pX data = deserialize_ZZ_pX(bytes);
  return kzg::poly(data);
}

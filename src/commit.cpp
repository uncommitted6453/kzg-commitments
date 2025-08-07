#include <kzg.h>
#include "util.h"

std::vector<uint8_t> kzg::commit::serialize() {
  return serialize_ECP(curve_point);
}

kzg::commit kzg::commit::deserialize(const std::vector<uint8_t>& bytes) {
  ECP point = deserialize_ECP(bytes);
  return kzg::commit(point);
}

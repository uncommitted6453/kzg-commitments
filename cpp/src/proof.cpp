#include <kzg.h>
#include "util.h"

std::vector<uint8_t> kzg::proof::serialize() {
  return serialize_ECP(curve_point);
}

kzg::proof kzg::proof::deserialize(const std::vector<uint8_t>& bytes) {
  ECP point = deserialize_ECP(bytes);
  return kzg::proof(point);
}

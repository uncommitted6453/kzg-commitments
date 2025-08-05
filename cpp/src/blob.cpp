#include "kzg.h"

kzg::blob kzg::blob::from_string(string s) {
  return kzg::blob::from_string(s, 0);
}

kzg::blob kzg::blob::from_string(string s, int offset) {
  vector<pair<ZZ_p, ZZ_p>> data;
  
  for (int i = 0; i < s.size(); i++) {
    ZZ_p ZZ_x, ZZ_y;
    ZZ_x = i + offset;
    ZZ_y = s[i];
    data.push_back({ ZZ_x, ZZ_y });
  }
  
  return kzg::blob(data); 
}

kzg::blob kzg::blob::from_bytes(const uint8_t* bytes, int offset, int size, int chunk) {
  if (chunk > MAX_CHUNK_SIZE)
    throw invalid_argument("chunk size is greater than MAX_CHUNK_SIZE");
  else if (offset % chunk != 0)
    throw invalid_argument("offset is not a multiple of chunk");
  else if (size % chunk != 0)
    throw invalid_argument("data size is not a multiple of chunk");
  
  vector<pair<ZZ_p, ZZ_p>> data;
  for (int i = offset / chunk; i < size / chunk; i++) {
    unsigned char chunk_data[MODBYTES_B160_56] = {0};
    for (int j = 0; j < chunk; j++)
      chunk_data[j] = bytes[i * chunk + j];
    
    ZZ chunk_scalar;
    ZZFromBytes(chunk_scalar, chunk_data, MODBYTES_B160_56);
    
    ZZ_p ZZ_x, ZZ_y;
    ZZ_x = i;
    ZZ_y = conv<ZZ_p>(chunk_scalar);
    
    data.push_back({ZZ_x, ZZ_y});
  }
  
  return kzg::blob(data);
}

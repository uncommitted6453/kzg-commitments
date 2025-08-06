#include <kzg.h>

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

kzg::blob kzg::blob::from_bytes(const uint8_t* bytes, int byte_offset, int byte_length, int chunk_size) {
  if (chunk_size > MAX_CHUNK_BYTES)
    throw invalid_argument("chunk_size must be lower than MAX_CHUNK_BYTES.");
  else if (byte_offset % chunk_size != 0)
    throw invalid_argument("byte_offset is not a multiple of chunk_size.");
  else if (byte_length % chunk_size != 0)
    throw invalid_argument("byte_length is not a multiple of chunk_size.");
  
  vector<pair<ZZ_p, ZZ_p>> data;
  for (int i = byte_offset / chunk_size; i < byte_length / chunk_size; i++) {
    unsigned char chunk_data[MODBYTES_CURVE] = {0};
    for (int j = 0; j < chunk_size; j++)
      chunk_data[j] = bytes[i * chunk_size + j];
    
    ZZ chunk_scalar;
    ZZFromBytes(chunk_scalar, chunk_data, MODBYTES_CURVE);
    
    ZZ_p ZZ_x, ZZ_y;
    ZZ_x = i;
    ZZ_y = conv<ZZ_p>(chunk_scalar);
    
    data.push_back({ZZ_x, ZZ_y});
  }
  
  return kzg::blob(data);
}

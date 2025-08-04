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

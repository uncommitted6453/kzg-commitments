#include <iostream>
#include <vector>
#include <kzg.h>
#include <fstream>
#include <chrono>
#include <iterator>

using namespace std::chrono;

void trusted_setup() {
  int num_coeffs = 65536;
  
  auto t_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(num_coeffs);
  auto t_stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(t_stop - t_start);
  cout << "KZG trusted setup generated in " << duration.count() / 1000000.0 << "s" << endl;
  cout << "  num_coeffs=" << num_coeffs << endl;
  cout << "  max_commit_bytes=" << num_coeffs * (kzg::CURVE_ORDER_BYTES - 1) << endl;
  
  kzg.export_setup();
}

int main(int argc, char *argv[]) {
  kzg::init();
  
  kzg::trusted_setup kzg("kzg_public");
  
  std::ifstream file("document.txt", std::ios::in | std::ios::binary);
  vector<char> data(
         (std::istreambuf_iterator<char>(file)),
         (std::istreambuf_iterator<char>()));
  vector<uint8_t> bytes(data.begin(), data.end());
  
  int chunk_size = (kzg::CURVE_ORDER_BYTES - 1);
  int zero_pad = chunk_size - (bytes.size() % chunk_size);
  
  for (int i = 0; i < zero_pad; i++)
    bytes.push_back(0);
  
  kzg::blob blob = kzg::blob::from_bytes(bytes.data(), 0, bytes.size(), chunk_size);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);
  
  vector<uint8_t> verify_bytes;
  for (int i = 0; i < chunk_size * 3; i++)
    verify_bytes.push_back(bytes[i]);
  
  kzg::proof proof = kzg.create_proof(poly, 0, 3);
  kzg::blob verify = kzg::blob::from_bytes(verify_bytes.data(), 0, verify_bytes.size(), chunk_size);
  if (kzg.verify_proof(commit, proof, verify)) cout << "verified: hello" << endl;
  
  return 0;
}

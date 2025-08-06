#include <iostream>
#include <vector>
#include <kzg.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iterator>

using namespace std::chrono;

void create_setup(int num_coeff) {
  auto t_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(num_coeff);
  auto t_stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(t_stop - t_start);

  cout << "KZG trusted setup generated in " << duration.count() / 1000000.0 << "s" << endl;
  cout << "  num_coeff=" << num_coeff << endl;
  cout << "  max_commit_bytes=" << num_coeff * MAX_CHUNK_BYTES << endl;
  
  kzg.export_setup();
}

string to_hex(vector<uint8_t>& bytes) {
  stringstream ss;
  for (uint8_t byte : bytes)
    ss << hex << setw(2) << setfill('0') << (unsigned int) byte;
  return ss.str();
}

void commit_file(string filename) {
  kzg::trusted_setup kzg("kzg_public");
  
  std::ifstream file(string(filename), std::ios::in | std::ios::binary);
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

  vector<uint8_t> commit_bytes = commit.serialize();
  cout << to_hex(commit_bytes) << endl;
}

void create_proof(string filename, int seed) {
  kzg::trusted_setup kzg("kzg_public");
  
  std::ifstream file(string(filename), std::ios::in | std::ios::binary);
  vector<char> data(
         (std::istreambuf_iterator<char>(file)),
         (std::istreambuf_iterator<char>()));
  vector<uint8_t> bytes(data.begin(), data.end());
  
  int zero_pad = MAX_CHUNK_BYTES - (bytes.size() % MAX_CHUNK_BYTES);
  
  for (int i = 0; i < zero_pad; i++)
    bytes.push_back(0);
  
  kzg::blob blob = kzg::blob::from_bytes(bytes.data(), 0, bytes.size(), MAX_CHUNK_BYTES);
  kzg::poly poly = kzg::poly::from_blob(blob);

  int chunk_length = data.size() / MAX_CHUNK_BYTES;
  int random_chunk = seed % (chunk_length - 4);

  kzg::proof proof = kzg.create_proof(poly, random_chunk, 4);
  
  vector<uint8_t> subsection_bytes;
  for (int i = random_chunk * MAX_CHUNK_BYTES; i < (random_chunk + 4) * MAX_CHUNK_BYTES; i++)
      subsection_bytes.push_back(bytes[i]);

  vector<uint8_t> proof_bytes = proof.serialize();
  cout << to_hex(proof_bytes) << " " << to_hex(subsection_bytes) << endl;
}

int main(int argc, char *argv[]) {
  kzg::init();
  
  if (string(argv[1]) == "setup") {
    create_setup(stoi(argv[2]));
  } else if (string(argv[1]) == "commit") {
    commit_file(string(argv[2]));
  } else if (string(argv[1]) == "prove") {
    create_proof(string(argv[2]), stoi(argv[3]));
  }
  
  return 0;
}

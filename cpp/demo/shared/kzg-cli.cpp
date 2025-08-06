#include <iostream>
#include <vector>
#include <kzg.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iterator>

using namespace std::chrono;

string to_hex(vector<uint8_t>& bytes) {
  stringstream ss;
  for (uint8_t byte : bytes)
    ss << hex << setw(2) << setfill('0') << (unsigned int) byte;
  return ss.str();
}

vector<uint8_t> from_hex(string s) {
  vector<uint8_t> res;
  for (int i = 0; i < s.size(); i += 2) {
    string hex_byte = s.substr(i, 2);
    res.push_back(strtol(hex_byte.c_str(), NULL, 16));
  }
  return res;
}

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

void commit_file(string filename) {
  kzg::trusted_setup kzg("../shared/kzg_public");
  
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
  kzg::trusted_setup kzg("../shared/kzg_public");
  
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
  cout << to_hex(proof_bytes) << " " << random_chunk << " " << to_hex(subsection_bytes) << endl;
}

int verify_proof(string commit_string, string proof_string, int chunk_offset, string data_string) {
  vector<uint8_t> commit_bytes = from_hex(commit_string);
  vector<uint8_t> proof_bytes = from_hex(proof_string);
  vector<uint8_t> data_bytes  = from_hex(data_string);

  kzg::trusted_setup kzg("../shared/kzg_public");
  
  kzg::commit commit = kzg::commit::deserialize(commit_bytes);
  kzg::proof proof = kzg::proof::deserialize(proof_bytes);

  int byte_offset = chunk_offset * MAX_CHUNK_BYTES;
  int byte_length = 4 * MAX_CHUNK_BYTES;

  kzg::blob verify = kzg::blob::from_bytes(data_bytes.data(), byte_offset, byte_length, MAX_CHUNK_BYTES);
  return kzg.verify_proof(commit, proof, verify) ? 0 : 1;
}

int main(int argc, char *argv[]) {
  kzg::init();
  
  if (string(argv[1]) == "setup") {
    create_setup(stoi(argv[2]));
  } else if (string(argv[1]) == "commit") {
    commit_file(string(argv[2]));
  } else if (string(argv[1]) == "prove") {
    create_proof(string(argv[2]), stoi(argv[3]));
  } else if (string(argv[1]) == "verify") {
    return verify_proof(string(argv[2]), string(argv[3]), stoi(argv[4]), string(argv[5]));
  }
  
  return 0;
}

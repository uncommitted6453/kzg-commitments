#include <iostream>
#include <kzg.h>
#include <chrono>
#include <random>
#include <string>
#include <iomanip>

using namespace std::chrono;

string generateRandomString(int length)
{
  const string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  random_device rd;
  mt19937 generator(rd());
  uniform_int_distribution<> distribution(0, characters.size() - 1);

  string random_string;
  random_string.reserve(length);

  for (int i = 0; i < length; ++i)
  {
    random_string += characters[distribution(generator)];
  }
  return random_string;
}

void benchmark_polynomial_degree(int max_degree) {
  cout << "Degree " << std::setw(4) << max_degree << ": ";
  
  // Measure setup time
  auto setup_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(max_degree);
  auto setup_end = high_resolution_clock::now();
  auto setup_duration = duration_cast<microseconds>(setup_end - setup_start);
  
  // Prepare test data
  string rand_str1 = generateRandomString(32);
  string rand_str2 = generateRandomString(32);
  kzg::blob blob = kzg::blob::from_string(rand_str1 + rand_str2);
  kzg::poly poly = kzg::poly::from_blob(blob);
  
  // Measure commit time
  auto commit_start = high_resolution_clock::now();
  kzg::commit commit = kzg.create_commit(poly);
  auto commit_end = high_resolution_clock::now();
  auto commit_duration = duration_cast<microseconds>(commit_end - commit_start);
  
  // Prepare verification data
  kzg::proof proof = kzg.create_proof(poly, 0, rand_str1.length());
  kzg::blob verify_target = kzg::blob::from_string(rand_str1, 0);
  
  // Measure verification time
  auto verify_start = high_resolution_clock::now();
  bool verification_result = kzg.verify_proof(commit, proof, verify_target);
  auto verify_end = high_resolution_clock::now();
  auto verify_duration = duration_cast<microseconds>(verify_end - verify_start);
  
  // Output results in compact format
  cout << "Setup: " << std::setw(6) << setup_duration.count() << "μs | "
       << "Commit: " << std::setw(5) << commit_duration.count() << "μs | "
       << "Verify: " << std::setw(5) << verify_duration.count() << "μs | "
       << (verification_result ? "✓" : "✗") << endl;
}

void benchmark_increasing_secret_bytes(int secret_bytes) {
  
  // Measure setup time
  auto setup_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(256, secret_bytes);
  auto setup_end = high_resolution_clock::now();
  auto setup_duration = duration_cast<microseconds>(setup_end - setup_start);
  cout << "Secret Bytes Setup" << std::setw(4) << secret_bytes << ": " << std::setw(6) << setup_duration.count() << "μs" << endl;
}

int main(int argc, char *argv[])
{
  kzg::init();
  
  // Test different polynomial degrees
  for (int degree = 128; degree <= 4096; degree *= 2) {
    benchmark_polynomial_degree(degree);
  }

  // Test different number of secret bytes
  for (int secret_bytes = 1; secret_bytes < MODBYTES_CURVE; secret_bytes += 4) {
    benchmark_increasing_secret_bytes(secret_bytes);
  }

  return 0;
}

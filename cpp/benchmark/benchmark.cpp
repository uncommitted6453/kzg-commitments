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

void benchmark_polynomial_max_degree(int max_degree)
{
  
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
  auto proof_start = high_resolution_clock::now();
  kzg::proof proof = kzg.create_proof(poly, 0, rand_str1.length());
  auto proof_end = high_resolution_clock::now();
  auto proof_duration = duration_cast<microseconds>(proof_end - proof_start);

  kzg::blob verify_target = kzg::blob::from_string(rand_str1, 0);
  
  // Measure verification time
  auto verify_start = high_resolution_clock::now();
  bool verification_result = kzg.verify_proof(commit, proof, verify_target);
  auto verify_end = high_resolution_clock::now();
  auto verify_duration = duration_cast<microseconds>(verify_end - verify_start);
  
  cout << "Max Degree: " << std::setw(4) << max_degree
       << " | Degree: " << std::setw(4) << 63
       << " | Setup: " << std::setw(6) << setup_duration.count() << "μs | "
       << "Commit: " << std::setw(5) << commit_duration.count() << "μs | "
       << "Proof: " << std::setw(5) << proof_duration.count() << "μs | "
       << "Verify: " << std::setw(5) << verify_duration.count() << "μs | "
       << (verification_result ? "✓" : "✗") << endl;
}

void benchmark_polynomial_degree(int degree)
{
  // Measure setup time
  auto setup_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(128);
  auto setup_end = high_resolution_clock::now();
  auto setup_duration = duration_cast<microseconds>(setup_end - setup_start);
  
  // Create a polynomial of specific degree by generating a string of length (degree + 1)
  // Since polynomial degree = number_of_points - 1
  string test_data = generateRandomString(degree + 1);
  kzg::blob blob = kzg::blob::from_string(test_data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  
  // Measure commit time
  auto commit_start = high_resolution_clock::now();
  kzg::commit commit = kzg.create_commit(poly);
  auto commit_end = high_resolution_clock::now();
  auto commit_duration = duration_cast<microseconds>(commit_end - commit_start);
  
  // Prepare verification data - use first half of the string for verification
  int verify_length = (degree + 1) / 2;
  auto proof_start = high_resolution_clock::now();
  kzg::proof proof = kzg.create_proof(poly, 0, verify_length);
  auto proof_end = high_resolution_clock::now();
  auto proof_duration = duration_cast<microseconds>(proof_end - proof_start);

  string verify_string = test_data.substr(0, verify_length);
  kzg::blob verify_target = kzg::blob::from_string(verify_string, 0);
  
  // Measure verification time
  auto verify_start = high_resolution_clock::now();
  bool verification_result = kzg.verify_proof(commit, proof, verify_target);
  auto verify_end = high_resolution_clock::now();
  auto verify_duration = duration_cast<microseconds>(verify_end - verify_start);
  
  cout << "Max Degree: " << std::setw(4) << 128
       << " | Degree: " << std::setw(3) << degree
       << " | Setup: " << std::setw(6) << setup_duration.count() << "μs | "
       << "Commit: " << std::setw(5) << commit_duration.count() << "μs | "
       << "Proof: " << std::setw(5) << proof_duration.count() << "μs | "
       << "Verify: " << std::setw(5) << verify_duration.count() << "μs | "
       << (verification_result ? "✓" : "✗") << endl;
}

int main(int argc, char *argv[])
{
  kzg::init();
  
  cout << "=== Benchmarking Trusted Setup Max Degree with degree=63 ===" << endl;
  // Test different polynomial max degrees
  for (int max_degree = 128; max_degree <= 4096; max_degree *= 2) {
    benchmark_polynomial_max_degree(max_degree);
  }

  cout << "\n=== Benchmarking Polynomial Degree with Max Degree=128 ===" << endl;
  // Test different polynomial degrees
  for (int degree = 10; degree <= 20; degree += 2) {
    benchmark_polynomial_degree(degree);
  }
  
  return 0;
}

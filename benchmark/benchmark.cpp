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

void benchmark_trusted_setup(int max_degree)
{
  // Measure setup time
  auto setup_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(max_degree);
  auto setup_end = high_resolution_clock::now();
  std::chrono::duration<double, std::milli> setup_duration = setup_end - setup_start;
  
  cout << "Terms: " << std::setw(4) << max_degree
       << " | Setup: " << std::setw(8) << std::fixed << std::setprecision(3) << setup_duration.count() << "ms"
       << endl;
}

void benchmark_single_proof(kzg::trusted_setup& kzg, int degree)
{
  string test_data = generateRandomString(degree + 1);
  kzg::blob blob = kzg::blob::from_string(test_data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  
  auto commit_start = high_resolution_clock::now();
  kzg::commit commit = kzg.create_commit(poly);
  auto commit_end = high_resolution_clock::now();
  std::chrono::duration<double, std::milli> commit_duration = commit_end - commit_start;
  
  auto proof_start = high_resolution_clock::now();
  kzg::proof proof = kzg.create_proof(poly, 0, 1);
  auto proof_end = high_resolution_clock::now();
  std::chrono::duration<double, std::milli> proof_duration = proof_end - proof_start;
  
  string verify_string = test_data.substr(0, 1);
  kzg::blob verify_target = kzg::blob::from_string(verify_string, 0);

  auto verify_start = high_resolution_clock::now();
  bool verification_result = kzg.verify_proof(commit, proof, verify_target);
  auto verify_end = high_resolution_clock::now();
  std::chrono::duration<double, std::milli> verify_duration = verify_end - verify_start;
  
  cout << "Degree: " << std::setw(8) << degree
       << " | Commit: " << std::setw(10) << std::fixed << std::setprecision(3) << commit_duration.count() << "ms | "
       << "Proof: " << std::setw(10) << std::fixed << std::setprecision(3) << proof_duration.count() << "ms | "
       << "Verify: " << std::setw(10) << std::fixed << std::setprecision(3) << verify_duration.count() << "ms | "
       << (verification_result ? "✓" : "✗") << endl;
}

void benchmark_multi_proof(kzg::trusted_setup& kzg, int num_proof)
{
  string test_data = generateRandomString(4096);
  kzg::blob blob = kzg::blob::from_string(test_data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  
  kzg::commit commit = kzg.create_commit(poly);
  
  // Prepare verification data - use first half of the string for verification
  int verify_length = num_proof;
  auto proof_start = high_resolution_clock::now();
  kzg::proof proof = kzg.create_proof(poly, 0, verify_length);
  auto proof_end = high_resolution_clock::now();
  std::chrono::duration<double, std::milli> proof_duration = proof_end - proof_start;

  string verify_string = test_data.substr(0, verify_length);
  kzg::blob verify_target = kzg::blob::from_string(verify_string, 0);
  
  // Measure verification time
  auto verify_start = high_resolution_clock::now();
  bool verification_result = kzg.verify_proof(commit, proof, verify_target);
  auto verify_end = high_resolution_clock::now();
  std::chrono::duration<double, std::milli> verify_duration = verify_end - verify_start;
  
  cout << "Degree: " << std::setw(3) << 4096
       << " | Proofs: " << std::setw(7) << num_proof
       << " | Proof: " << std::setw(7) << std::fixed << std::setprecision(3) << proof_duration.count() << "ms | "
       << "Verify: " << std::setw(7) << std::fixed << std::setprecision(3) << verify_duration.count() << "ms | "
       << (verification_result ? "✓" : "✗") << endl;
}

int main(int argc, char *argv[])
{
  kzg::init();
  
  cout << "=== Benchmarking Trusted Setup ===" << endl;
  for (int max_degree = 128; max_degree <= 4096; max_degree *= 2) {
    benchmark_trusted_setup(max_degree);
  }
  
  kzg::trusted_setup kzg(5000);

  cout << "\n=== Benchmarking Single Proofs ===" << endl;
  for (int degree = 128; degree <= 4096; degree *= 2) {
    benchmark_single_proof(kzg, degree);
  }
  
  cout << "\n=== Benchmarking Multi Proofs ===" << endl;
  for (int num_proof = 128; num_proof <= 4096; num_proof *= 2) {
    benchmark_multi_proof(kzg, num_proof);
  }
  
  if (argc > 1 && string(argv[1]) == "--benchmark-common") {
    cout << "\n=== Benchmark Common ===" << endl;

    cout << "Loading large trusted setup..." << endl;
    auto setup_start = high_resolution_clock::now();
    kzg = kzg::trusted_setup(10429000);
    auto setup_end = high_resolution_clock::now();
    auto setup_duration = setup_end - setup_start;
    cout << "Finished loading setup in " << setup_duration.count() / 1000000000.0 << "s" << endl;
    
    for (int degree = 1024; degree <= 10428576; degree *= 2) {
      benchmark_single_proof(kzg, degree);
    }
  }
  
  return 0;
}

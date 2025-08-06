#include <iostream>
#include <vector>
#include <kzg.h>
#include <chrono>
#include <random>
#include <string>

using namespace std::chrono;

string generateRandomString(double length)
{
  const string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  random_device rd;
  mt19937 generator(rd());
  uniform_int_distribution<> distribution(0, characters.size() - 1);

  string random_string;
  random_string.reserve(length);

  for (double i = 0; i < length; ++i)
  {
    random_string += characters[distribution(generator)];
  }
  return random_string;
};

void data_benchmark()
{
  kzg::trusted_setup kzg(128);

  vector<double> sizes = {16, 32, 64, 128, 256};
  for (double size : sizes)
  {
    string data = generateRandomString(size);

    auto t_start = high_resolution_clock::now();

    kzg::blob blob = kzg::blob::from_string(data);
    kzg::poly poly = kzg::poly::from_blob(blob);
    kzg::commit commit = kzg.create_commit(poly);

    auto t_stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t_stop - t_start);
    cout << "commit took " << duration.count() << " microseconds for size " << size << endl;
  }
}

void setup_benchmark()
{
  vector<double> sizes = {128, 256, 512, 1024, 2048};
  for (double size : sizes)
  {
    auto t_start = high_resolution_clock::now();
    kzg::trusted_setup kzg(size);
    auto t_stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t_stop - t_start);
    cout << "setup took " << duration.count() << " microseconds for size " << size << endl;
  }
}

void verification_benchmark()
{
  kzg::trusted_setup kzg(128);

  vector<double> sizes = {16, 32, 64, 110, 256};
  for (double size : sizes)
  {
    string rand_str1 = generateRandomString(size);
    string rand_str2 = generateRandomString(size);

    string data = rand_str1 + rand_str2;
    kzg::blob blob = kzg::blob::from_string(data);
    kzg::poly poly = kzg::poly::from_blob(blob);
    kzg::commit commit = kzg.create_commit(poly);

    auto t_start = high_resolution_clock::now();

    // create proof for first rand_str.length() chars
    kzg::proof hello_proof = kzg.create_proof(poly, 0, rand_str1.length());
    // verify committed message starts with rand_str1
    kzg::blob verify_target = kzg::blob::from_string(rand_str1, 0);
    if (kzg.verify_proof(commit, hello_proof, verify_target)) {
      auto t_stop = high_resolution_clock::now();
      auto duration = duration_cast<microseconds>(t_stop - t_start);
      cout << "verification took " << duration.count() << " microseconds for size " << size << endl;
    }
    else {
      cout << "verification failed for size" << size << "!" << endl;
    }
  }
}

int main(int argc, char *argv[])
{
  kzg::init();

  data_benchmark();
  setup_benchmark();
  verification_benchmark();

  return 0;
}

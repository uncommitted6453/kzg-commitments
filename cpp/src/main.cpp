#include <iostream>
#include <vector>
#include "kzg.h"
#include "util.h"
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace NTL;

void example_program()
{
  /* ----- setup ------ */
  kzg::trusted_setup kzg(128);

  /* ----- commit ------ */
  string data = "hello there my name is bob";
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);

  if (kzg.verify_commit(commit, poly))
    cout << "verified: commit is correct" << endl;

  /* ----- create proof------ */
  kzg::proof hello_proof = kzg.create_proof(poly, 0, strlen("hello"));
  kzg::proof name_proof = kzg.create_proof(poly, strlen("hello there my "), strlen("name is"));
  kzg::proof bob_proof = kzg.create_proof(poly, strlen("hello there my name is "), strlen("bob"));

  /* ------ verify ------- */
  kzg::blob verify = kzg::blob::from_string("hello", 0);
  if (kzg.verify_proof(commit, hello_proof, verify))
    cout << "verified: hello" << endl;

  verify = kzg::blob::from_string("name is", strlen("hello there my "));
  if (kzg.verify_proof(commit, name_proof, verify))
    cout << "verified: name is" << endl;

  verify = kzg::blob::from_string("bob", strlen("hello there my name is "));
  if (kzg.verify_proof(commit, bob_proof, verify))
    cout << "verified: bob" << endl;

  verify = kzg::blob::from_string("alice", strlen("hello there my name is "));
  if (!kzg.verify_proof(commit, bob_proof, verify))
    cout << "verified: not alice" << endl;
}

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
  auto t_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(128);
  auto t_stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(t_stop - t_start);
  cout << "setup took " << duration.count() << " microseconds" << endl;

  vector<double> sizes = {16, 32, 64, 128, 256};
  for (double size : sizes)
  {
    string data = generateRandomString(size);

    t_start = high_resolution_clock::now();

    kzg::blob blob = kzg::blob::from_string(data);
    kzg::poly poly = kzg::poly::from_blob(blob);
    kzg::commit commit = kzg.create_commit(poly);

    t_stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(t_stop - t_start);
    cout << "commit took " << duration.count() << " microseconds for size " << size << endl;
  }
}

int main(int argc, char *argv[])
{
  example_program();
  data_benchmark();
  return 0;
}
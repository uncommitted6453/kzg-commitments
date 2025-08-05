#include <iostream>
#include <vector>
#include <kzg.h>
#include <chrono>


using namespace std::chrono;

void example_program() {
  /* ----- setup ------ */
  kzg::trusted_setup kzg(128);
  
  /* ----- commit ------ */
  string data = "hello there my name is bob";
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);
  
  if (kzg.verify_commit(commit, poly)) cout << "verified: commit is correct" << endl;
  
  /* ----- create proof------ */
  kzg::proof hello_proof = kzg.create_proof(poly, 0, strlen("hello"));
  kzg::proof name_proof = kzg.create_proof(poly, strlen("hello there my "), strlen("name is"));
  kzg::proof bob_proof = kzg.create_proof(poly, strlen("hello there my name is "), strlen("bob"));
  
  /* ------ verify ------- */
  kzg::blob verify = kzg::blob::from_string("hello", 0);
  if (kzg.verify_proof(commit, hello_proof, verify)) cout << "verified: hello" << endl;
  
  verify = kzg::blob::from_string("name is", strlen("hello there my "));
  if (kzg.verify_proof(commit, name_proof, verify)) cout << "verified: name is" << endl;
  
  verify = kzg::blob::from_string("bob", strlen("hello there my name is "));
  if (kzg.verify_proof(commit, bob_proof, verify)) cout << "verified: bob" << endl;
  
  verify = kzg::blob::from_string("alice", strlen("hello there my name is "));
  if (!kzg.verify_proof(commit, bob_proof, verify)) cout << "verified: not alice" << endl;
}

void example_benchmark() {
  auto t_start = high_resolution_clock::now();
  kzg::trusted_setup kzg(128);
  auto t_stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(t_stop - t_start);
  cout << "setup took " << duration.count() << " microseconds" << endl;
  
  t_start = high_resolution_clock::now();
  
  string data = "abcdefghijklmnopqrstuvxyz";
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);
  
  t_stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(t_stop - t_start);
  cout << "commit took " << duration.count() << " microseconds" << endl;
}

void example_chunking() {
  kzg::trusted_setup kzg(128);
  
  kzg::blob unchunked_blob = kzg::blob::from_string("hello there my name is bob.");
  kzg::poly unchunked_poly = kzg::poly::from_blob(unchunked_blob);
  cout << "unchunked polynomial is degree " << deg(unchunked_poly.get_poly()) << endl;
  
  unsigned char data[] = "hello there my name is bob.";
  kzg::blob blob = kzg::blob::from_bytes(data, 0, sizeof(data), 4);
  kzg::poly poly = kzg::poly::from_blob(blob);
  kzg::commit commit = kzg.create_commit(poly);
  cout << "chunked polynomial (4 bytes in a coefficient) is degree " << deg(poly.get_poly()) << endl;
  
  if (kzg.verify_commit(commit, poly)) cout << "verified: commit is correct" << endl;
  
  kzg::proof proof = kzg.create_proof(poly, 0, 8, 4);
  
  kzg::blob verify = kzg::blob::from_bytes((uint8_t*) "hello th", 0, 8, 4);
  if (kzg.verify_proof(commit, proof, verify)) cout << "verified: hello" << endl;
}

void example_serialize() {
  kzg::trusted_setup kzg(128);
  
  string data = "hello there my name is bob";
  kzg::blob blob = kzg::blob::from_string(data);
  kzg::poly poly = kzg::poly::from_blob(blob);

  for (int i = 0; i < deg(poly.get_poly()); i++)
      cout << poly.get_poly()[i] << endl;

  cout << "vvvvvvv" << endl;
  
  vector<uint8_t> bytes = poly.serialize();

  kzg::poly poly2 = kzg::poly::deserialize(bytes);
  for (int i = 0; i < deg(poly2.get_poly()); i++)
      cout << poly2.get_poly()[i] << endl;
}

int main(int argc, char *argv[]) {
  kzg::init();
  
  example_program();
  cout << "--------------------------------------------" << endl;
  example_benchmark();
  cout << "--------------------------------------------" << endl;
  example_chunking();
  cout << "--------------------------------------------" << endl;
  example_serialize();
  return 0;
}

# kzg-commitments
A C++ library implementing the KZG (Kate-Zaverucha-Goldberg) commitment scheme.

# Table of Contents
- [Documentation](#documentation)
- [Installation](#installation)
- [Getting Started](#getting-started)
- [Testing](#testing)
- [Benchmarking](#benchmarking)

# Documentation

https://uncommitted6453.github.io/kzg-commitments/index.html

# Installation

Run the following to build the library:

```sh
git clone --recurse-submodules -j8 https://github.com/uncommitted6453/kzg-commitments.git
cd kzg-commitments/cpp
make
```

Then, copy the contents of the generated lib and include folders into your project and link them when compiling 
with your toolchain. For example, with g++, this might look like:
```cpp
g++ my-project.cpp -Iinclude lib/kzg-bn254.a lib/core.a lib/ntl.a -lgmp -o a.out
```
# Getting Started

Here is some sample code for using the kzg-commitments library:

```c++
// As KZG is a polynomial commitment scheme, You must encode your data into
// a polynomial first using blob::from_string or blob::from_data
string data = "hello there my name is bob";
kzg::blob blob = kzg::blob::from_string(data);
kzg::poly poly = kzg::poly::from_blob(blob);

// Create a commitment to your polynomial. 
kzg::commit commit = kzg.create_commit(poly);

// You can verify a commit matches a polynomial
if (kzg.verify_commit(commit, poly))
  cout << "verified: commit is correct" << endl;

// Or, as KZG allows for partial reveals,  you can create proofs for portions of the data
kzg::proof hello_proof = kzg.create_proof(poly, 0, strlen("hello"));

// And verify proofs
kzg::blob verify = kzg::blob::from_string("hello", 0);
if (kzg.verify_proof(commit, hello_proof, verify))
  cout << "verified: hello" << endl;
```

The output will be as follows:

```
verified: commit is correct
verified: hello
```

# Testing

The tests will be run automatically upon building the program:

```sh
cd kzg-commitments/cpp
make
```

but you can also run the program manually after building with 

```sh
cd kzg-commitments/cpp/testing
./testing
```

# Benchmarking

Run the benchmarks yourself by running the following commands:

```sh
cd cpp
./benchmark_curves.sh
```

Our results were as follows:
```
bn158:

=== Benchmarking Trusted Setup ===
Terms:  128 | Setup:   18.293ms
Terms:  256 | Setup:   31.476ms
Terms:  512 | Setup:   62.800ms
Terms: 1024 | Setup:  133.105ms
Terms: 2048 | Setup:  246.527ms
Terms: 4096 | Setup:  482.306ms

=== Benchmarking Single Proofs ===
Degree:      128 | Commit:     11.612ms | Proof:     11.608ms | Verify:      1.847ms | ✓
Degree:      256 | Commit:     26.548ms | Proof:     23.826ms | Verify:      2.898ms | ✓
Degree:      512 | Commit:     48.153ms | Proof:     48.852ms | Verify:      3.471ms | ✓
Degree:     1024 | Commit:    100.149ms | Proof:     90.389ms | Verify:      3.337ms | ✓
Degree:     2048 | Commit:    219.755ms | Proof:    194.226ms | Verify:      1.633ms | ✓
Degree:     4096 | Commit:    378.268ms | Proof:    404.283ms | Verify:      1.701ms | ✓

=== Benchmarking Multi Proofs ===
Degree: 4096 | Proofs:     128 | Proof: 459.939ms | Verify:  44.586ms | ✓
Degree: 4096 | Proofs:     256 | Proof: 408.241ms | Verify: 101.035ms | ✓
Degree: 4096 | Proofs:     512 | Proof: 361.574ms | Verify: 181.904ms | ✓
Degree: 4096 | Proofs:    1024 | Proof: 366.182ms | Verify: 377.419ms | ✓
Degree: 4096 | Proofs:    2048 | Proof: 427.972ms | Verify: 806.526ms | ✓
Degree: 4096 | Proofs:    4096 | Proof: 571.068ms | Verify: 1646.688ms | ✓

bn254:

=== Benchmarking Trusted Setup ===
Terms:  128 | Setup:   34.207ms
Terms:  256 | Setup:   71.426ms
Terms:  512 | Setup:  169.810ms
Terms: 1024 | Setup:  287.471ms
Terms: 2048 | Setup:  598.011ms
Terms: 4096 | Setup:  947.288ms

=== Benchmarking Single Proofs ===
Degree:      128 | Commit:     30.083ms | Proof:     42.504ms | Verify:      3.269ms | ✓
Degree:      256 | Commit:     55.151ms | Proof:     62.676ms | Verify:      3.259ms | ✓
Degree:      512 | Commit:    136.220ms | Proof:    122.255ms | Verify:      3.082ms | ✓
Degree:     1024 | Commit:    211.453ms | Proof:    267.592ms | Verify:      3.109ms | ✓
Degree:     2048 | Commit:    445.452ms | Proof:    446.845ms | Verify:      3.718ms | ✓
Degree:     4096 | Commit:   1104.637ms | Proof:   1080.747ms | Verify:      3.109ms | ✓

=== Benchmarking Multi Proofs ===
Degree: 4096 | Proofs:     128 | Proof: 922.247ms | Verify:  94.114ms | ✓
Degree: 4096 | Proofs:     256 | Proof: 860.305ms | Verify: 187.539ms | ✓
Degree: 4096 | Proofs:     512 | Proof: 810.811ms | Verify: 369.300ms | ✓
Degree: 4096 | Proofs:    1024 | Proof: 800.158ms | Verify: 814.464ms | ✓
Degree: 4096 | Proofs:    2048 | Proof: 745.346ms | Verify: 1599.394ms | ✓

bls12381:

=== Benchmarking Trusted Setup ===
Terms:  128 | Setup:   45.303ms
Terms:  256 | Setup:  109.405ms
Terms:  512 | Setup:  144.887ms
Terms: 1024 | Setup:  284.286ms
Terms: 2048 | Setup:  570.938ms
Terms: 4096 | Setup: 1008.991ms

=== Benchmarking Single Proofs ===
Degree:      128 | Commit:     38.794ms | Proof:     44.258ms | Verify:      9.437ms | ✓
Degree:      256 | Commit:     73.779ms | Proof:     94.335ms | Verify:      9.284ms | ✓
Degree:      512 | Commit:    151.911ms | Proof:    143.107ms | Verify:     16.916ms | ✓
Degree:     1024 | Commit:    314.844ms | Proof:    339.103ms | Verify:      9.383ms | ✓
Degree:     2048 | Commit:    610.632ms | Proof:    616.749ms | Verify:      9.117ms | ✓
Degree:     4096 | Commit:   1153.631ms | Proof:   1399.958ms | Verify:      9.035ms | ✓

=== Benchmarking Multi Proofs ===
Degree: 4096 | Proofs:     128 | Proof: 1358.254ms | Verify: 134.653ms | ✓
Degree: 4096 | Proofs:     256 | Proof: 1118.744ms | Verify: 242.510ms | ✓
Degree: 4096 | Proofs:     512 | Proof: 1077.430ms | Verify: 480.500ms | ✓
Degree: 4096 | Proofs:    1024 | Proof: 993.971ms | Verify: 995.959ms | ✓
Degree: 4096 | Proofs:    2048 | Proof: 874.976ms | Verify: 2032.417ms | ✓


```

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
bls12381:

=== Benchmarking Trusted Setup Max Degree with degree=63 ===
Max Degree:  128 | Degree:   63 | Setup:   42.609ms | Commit:  29.532ms | Proof:   9.773ms | Verify:  73.119ms | ✓
Max Degree:  256 | Degree:   63 | Setup:   79.061ms | Commit:  19.870ms | Proof:  10.300ms | Verify:  66.339ms | ✓
Max Degree:  512 | Degree:   63 | Setup:  152.746ms | Commit:  20.967ms | Proof:  10.026ms | Verify:  58.715ms | ✓
Max Degree: 1024 | Degree:   63 | Setup:  287.988ms | Commit:  18.783ms | Proof:   9.965ms | Verify:  60.944ms | ✓
Max Degree: 2048 | Degree:   63 | Setup:  544.456ms | Commit:  18.265ms | Proof:   9.632ms | Verify:  63.450ms | ✓
Max Degree: 4096 | Degree:   63 | Setup: 1064.539ms | Commit:  18.745ms | Proof:  10.520ms | Verify:  55.507ms | ✓

=== Benchmarking Polynomial Degree with Max Degree=128 ===
Max Degree:  128 | Degree:  10 | Setup:   37.529ms | Commit:   3.270ms | Proof:   1.733ms | Verify:  15.578ms | ✓
Max Degree:  128 | Degree:  12 | Setup:   39.445ms | Commit:   3.884ms | Proof:   1.997ms | Verify:  16.520ms | ✓
Max Degree:  128 | Degree:  14 | Setup:   40.112ms | Commit:   4.448ms | Proof:   2.759ms | Verify:  19.257ms | ✓
Max Degree:  128 | Degree:  16 | Setup:   37.554ms | Commit:   5.018ms | Proof:   2.566ms | Verify:  20.008ms | ✓
Max Degree:  128 | Degree:  18 | Setup:   39.387ms | Commit:   5.573ms | Proof:   2.945ms | Verify:  22.771ms | ✓
Max Degree:  128 | Degree:  20 | Setup:   36.344ms | Commit:   6.584ms | Proof:   3.207ms | Verify:  23.105ms | ✓

bn158:

=== Benchmarking Trusted Setup Max Degree with degree=63 ===
Max Degree:  128 | Degree:   63 | Setup:   22.337ms | Commit:   6.495ms | Proof:   3.410ms | Verify:  14.101ms | ✓
Max Degree:  256 | Degree:   63 | Setup:   38.914ms | Commit:   5.539ms | Proof:   3.852ms | Verify:  15.468ms | ✓
Max Degree:  512 | Degree:   63 | Setup:  122.122ms | Commit:  28.256ms | Proof:  16.930ms | Verify:  24.721ms | ✓
Max Degree: 1024 | Degree:   63 | Setup:  140.967ms | Commit:   5.810ms | Proof:   3.505ms | Verify:  12.115ms | ✓
Max Degree: 2048 | Degree:   63 | Setup:  249.029ms | Commit:   5.681ms | Proof:   3.810ms | Verify:  11.386ms | ✓
Max Degree: 4096 | Degree:   63 | Setup:  488.010ms | Commit:   5.375ms | Proof:   3.318ms | Verify:  17.117ms | ✓

=== Benchmarking Polynomial Degree with Max Degree=128 ===
Max Degree:  128 | Degree:  10 | Setup:   15.714ms | Commit:   0.857ms | Proof:   0.640ms | Verify:   2.998ms | ✓
Max Degree:  128 | Degree:  12 | Setup:   16.792ms | Commit:   1.861ms | Proof:   1.184ms | Verify:   5.787ms | ✓
Max Degree:  128 | Degree:  14 | Setup:   16.956ms | Commit:   1.229ms | Proof:   0.837ms | Verify:   6.622ms | ✓
Max Degree:  128 | Degree:  16 | Setup:   17.270ms | Commit:   1.373ms | Proof:   0.834ms | Verify:   3.585ms | ✓
Max Degree:  128 | Degree:  18 | Setup:   17.162ms | Commit:   1.597ms | Proof:   1.029ms | Verify:   3.855ms | ✓
Max Degree:  128 | Degree:  20 | Setup:   16.579ms | Commit:   2.329ms | Proof:   1.047ms | Verify:   4.638ms | ✓

bn254:

=== Benchmarking Trusted Setup Max Degree with degree=63 ===
Max Degree:  128 | Degree:   63 | Setup:   49.847ms | Commit:  21.263ms | Proof:  13.238ms | Verify:  36.026ms | ✓
Max Degree:  256 | Degree:   63 | Setup:   89.127ms | Commit:  17.687ms | Proof:  10.893ms | Verify:  25.884ms | ✓
Max Degree:  512 | Degree:   63 | Setup:  149.628ms | Commit:  13.754ms | Proof:   7.404ms | Verify:  25.308ms | ✓
Max Degree: 1024 | Degree:   63 | Setup:  276.698ms | Commit:  14.994ms | Proof:  15.513ms | Verify:  35.544ms | ✓
Max Degree: 2048 | Degree:   63 | Setup:  659.569ms | Commit:  13.786ms | Proof:   7.275ms | Verify:  25.541ms | ✓
Max Degree: 4096 | Degree:   63 | Setup: 1327.310ms | Commit:  20.603ms | Proof:   9.743ms | Verify:  33.519ms | ✓

=== Benchmarking Polynomial Degree with Max Degree=128 ===
Max Degree:  128 | Degree:  10 | Setup:   42.114ms | Commit:   3.641ms | Proof:   2.248ms | Verify:   5.979ms | ✓
Max Degree:  128 | Degree:  12 | Setup:   36.769ms | Commit:   3.563ms | Proof:   1.527ms | Verify:   5.883ms | ✓
Max Degree:  128 | Degree:  14 | Setup:   34.236ms | Commit:   3.924ms | Proof:   2.914ms | Verify:   6.866ms | ✓
Max Degree:  128 | Degree:  16 | Setup:   33.216ms | Commit:   3.491ms | Proof:   2.105ms | Verify:   8.837ms | ✓
Max Degree:  128 | Degree:  18 | Setup:   33.771ms | Commit:   3.770ms | Proof:   2.134ms | Verify:   7.453ms | ✓
Max Degree:  128 | Degree:  20 | Setup:   32.438ms | Commit:   4.565ms | Proof:   3.212ms | Verify:  14.750ms | ✓
```

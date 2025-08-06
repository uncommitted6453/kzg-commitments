# kzg-commitments

## Benchmarking

Run the benchmarks yourself by running the following commands:

```sh
cd cpp
./benchmark_curves.sh
```

Our results were as follows:
```
bls12381:

Degree  128: Setup:  42087μs | Commit: 23241μs | Verify: 58953μs | ✓
Degree  256: Setup:  67752μs | Commit: 17842μs | Verify: 54024μs | ✓
Degree  512: Setup: 133184μs | Commit: 22407μs | Verify: 79773μs | ✓
Degree 1024: Setup: 267437μs | Commit: 17497μs | Verify: 53364μs | ✓
Degree 2048: Setup: 527026μs | Commit: 24516μs | Verify: 53314μs | ✓
Degree 4096: Setup: 1006543μs | Commit: 17491μs | Verify: 52351μs | ✓

bn158:

Degree  128: Setup:  16672μs | Commit:  8324μs | Verify: 12368μs | ✓
Degree  256: Setup:  31686μs | Commit:  5524μs | Verify: 12632μs | ✓
Degree  512: Setup:  63883μs | Commit:  5164μs | Verify: 12082μs | ✓
Degree 1024: Setup: 123140μs | Commit:  5190μs | Verify: 11693μs | ✓
Degree 2048: Setup: 345944μs | Commit: 18715μs | Verify: 13912μs | ✓
Degree 4096: Setup: 501990μs | Commit:  5050μs | Verify: 10730μs | ✓

bn254:

Degree  128: Setup:  33793μs | Commit: 12492μs | Verify: 25598μs | ✓
Degree  256: Setup:  63233μs | Commit: 12621μs | Verify: 22524μs | ✓
Degree  512: Setup: 130900μs | Commit: 15559μs | Verify: 26902μs | ✓
Degree 1024: Setup: 219164μs | Commit: 12562μs | Verify: 22778μs | ✓
Degree 2048: Setup: 464625μs | Commit: 12637μs | Verify: 23251μs | ✓
Degree 4096: Setup: 868927μs | Commit: 12556μs | Verify: 22705μs | ✓
```

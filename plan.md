# Response to Abstract Feedback

benchmarking section should only cover performance and space, and comparing to other schemes
testing security properties should be done in cryptanalysis

> What sort of cryptanalysis?

- note and give a brief overview of the security proofs in the kzg paper as well as assumptions
- do research on existing investigations into kzg or commmitment scheme security
- empirically verify the security
  - write a script which naively demonstrates the hiding and binding property of kzg
    - binding:
      - select some message and secret to commit and produce a proof
      - generate random messages and show that messages that aren't the
        original message generally don't satisfy the proof
    - hiding:
      - fix a message and show that the distribution of proofs from random secrets is uniform
      - show the above over various messages
- investgiate KZG's different parameters and how weaker parameters allow for attacks
  - security of cyclic groups used
    - finite field over integers
    - finite field over elliptic curves, explore different types of curves
    - group order

> Benchmarking, which commitment schemes to compare against?
  - naive vector hashing
  - merkle tree
  - how is it different to KZG? (performance in speed,space,security)

# Optimization

- some optimizations noted in the paper
  - reuse the shared pk over multiple commitments
  - parallelize generating each exponention component
  - use fast exponentation techniques instead of naive implementation
  - precompute e(C,g) and e(g,g) during verification

- encode multiple words into a single polynomial value
  - note that the order of the curve is extremely large
  - then a very large number can encode multiple ascii characters
  - then the polynomial degree doesn't need to be as high
- when calculating powers of g for the trusted setup, i think we can reuse previous results
  - e.g. g^10 = g^(7 + 3) = g^7 * g^3
  - TODO: verify the mathematical validity of this, esp over elliptic curves.
  - we may combine this with parallelization for very large powers.
  - \*\* the paper mentions using fast exponention when computing the polynomial not sure if this is the exact same concept, we're using apporach since the trusted setup requires all powers of g to be stored.

# Further Research

- [Formal Verification of KZG scheme](https://fcs-workshop.github.io/fcs2024/papers/FCS_Rothmann_Kreuzer.pdf)
- [PBC library](https://crypto.stanford.edu/pbc/)
- [MIRACL Core](https://github.com/miracl/core)

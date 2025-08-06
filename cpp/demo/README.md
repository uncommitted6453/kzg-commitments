# Demo

Let us consider a demonstrative use of KZG in the context of trustless
distributed systems, specifically, involving blockchain and automatic
contracts.

Consider a simple hypothetical distributed storage protocol. Peers host other
peer's files and are incetivised into doing so via digital currency payment. To
facilitate the tracking of such exchanges, we suppose a standard trustless and
decentralised ledger is used, such as blockchain. Exchanges are recorded onto
the blockchain, which is assumed to be immutable. Balance is also exclusively
calculated via exchanges published on the chain. We further suppose that there
already exists a shared trusted KZG setup between each host.

## Protocol

We describe the protocol via a hypothetical exhcange between three peers, A, B
and Q:
- Peer A wants a file, document.txt, to be hosted on other machines over a period of time so does the following:
  - Peer A generates a commit of the entire file.
  - Peer A publishes a contract on the ledger to the following effect:
    - Here is a commit, C, of the file `document.txt`.
    - Here is a signed transaction of balance.
    - If you can post proofs for random sections (dictated by external factors)
      of `document.txt`, every hour, for the next 24 hours then you gain
      ownership of this transaction.
- Peer B accepts this contract and downloads `document.txt` from B.
- Every hour peer B generates a proof for a random section of `document.txt` and publishes it onto the ledger.
- After the 24 hours, peer B fufills the contract.
- Now peer B wants to buy something from peer Q.
- Peer B cites his fufillment of peer A's contract as a proof of balance
- Peer Q verifies B's proof of balance from the ledger and accepts the transaction.

## Correctness

We note that since peer B must publish random sections of the file at any given
time, he must effectively always hold the file or otherwise predict which
random section he must publish. Furthermore, creating a proof of the commit
requires the full committed polynomial, which is essentially equivalent to
holding the file.

Then to any third party viewing the ledger, peer B being able to publish the commits
demonstrates that he fufilled the contract.

## Motivation

In addition, certain KZG outputs are constant size, specifically the commit and the proof.
In this case, the commit and proof are published onto the ledger. The commit is constant size,
so a contract published on the ledger does not scale with the size of the hosted file.
And while a third party requires the proof values to verify the proof, since
the proof is only of subsections of the file, this can be a small fixed size
section which similarly does not scale in relation to file size.

Hence, the use of KZG in this exchange correctly and efficiently satisfies the protocol requirements.

## Technical Demo

We implement a simplified demonstration of this protocol.
The peer actions are defined through bash scripts in their respective files.
In shared, the `kzg-cli.cpp` program provides an executable CLI interface which bash scritps can interface with.
To run this demo, ensure that `kzg-cli` has been built by running `make` in the root directory.
Then to run the `./run-demo` script.

## Details

The `ledger-publish`, `ledger-read`, `ledger-top` scripts provide an interface
for each peer to interface with a ledger. In this case, for the sake of
simplcitiy, the ledger operations simpley write or read to a file, named by the block number.
To view the contents of the ledger, you can view the files in `shared/ledger`.

The data in each ledger block are loosely defined to simply provide an example for our demonstrations.
For clarity, we will define the arguments in each block type.

```
host_request [file-name] [file-kzg-commit] [hosting-period] [proof-of-balance]
  A peer is reqests other peers to download the specified file and host it for
  distribution over the given period of time. Since peers are compensated are
  doing so, the requestee must stake some proof of balance to be transferred
  for this contract.
random [seed]
  This generates a distributed and agreed upon source of randomness which also tracks time.
  To prove that an operation was performed at a certain time, and not pre-generated locally,
  data should agree with this value.
file_proof [host-request-block] [section-kzg-proof] [chunk-offset] [chunk-data]
  Using the previously defined random seed, the peer fufilling the host
  request, determines an agreed upon subsection of the file the hostee must
  prove they have.
transfer-to [peer] using [host-request-block] proof [file_proof-blocks]+
  A peer can pay another peer by citing their fufillment of a request as a proof of balance.
  Using the properties of KZG, another peer can verify the contract has been
  fufilled without even without access to the entirety of the file.
```

# Demo

Let us consider a demonstrative use of KZG in the context of trustless
distributed systems, specifically, involving blockchain and automatic
contracts.

Consider a hypothetical distributed storage protocol. Peers host other peer's
files and are incetivised into doing so via digital currency payment. To
facilitate the tracking of such exchanges, we suppose a standard trustless and
decentralised ledger is used, such as blockchain. Exchanges are recorded onto
the blockchain, which is assumed to be immutable. Balance is also exclusively
calculated via exchanges published on the chain. We further suppose that there
already exists a shared trusted KZG setup between each host.

We describe the protocol via a hypothetical exhcange between three peers, A, B
and C:
- Peer A wants a file, document.txt, to be hosted on other machines over a
  period of time so does the following:
  - Peer A generates a commit of the entire file
  - Peer A publishes a contract on the ledger to the following effect:
  Here is a commit of 
- 

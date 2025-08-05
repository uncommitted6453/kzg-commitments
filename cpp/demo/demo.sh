#!/bin/sh

./personA/post_contract
sleep 1

./personB/accept_contract
sleep 1

for i in {1..5}; do
  ./personC/create_noise
  sleep 1
  ./personC/create_noise
  sleep 1
  ./personB/create_proof
  sleep 1
done

./personC/check_balance

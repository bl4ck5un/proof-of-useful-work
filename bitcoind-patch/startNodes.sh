#!/bin/bash 

mv node1 "/tmp/`date` node1"
mv node2 "/tmp/`date` node2"

mkdir node1 
mkdir node2 

./src/bitcoind -datadir=node1 -conf=../node1.conf &
./src/bitcoind -datadir=node2 -conf=../node2.conf &

# ./src/bitcoind -datadir=/home/ittay/files/workspace/bitcoin-pouw/data1 -conf=../node1.conf &
# ./src/bitcoind -datadir=/home/ittay/files/workspace/bitcoin-pouw/data2 -conf=../node2.conf &

#!/bin/bash
set -euxo pipefail

SNAPSHOT=$(curl -s --max-redirs 0 https://api.mainnet-beta.solana.com/snapshot.tar.bz2)
sleep 2
wget -nc --trust-server-names https://api.mainnet-beta.solana.com/snapshot.tar.bz2 -P /data/firedancer/ledger

rm -f /data/firedancer/ledger/mainnet-funk
./build/native/gcc/bin/fd_ledger \
  --cmd ingest \
  --page-cnt 32 \
  --funk-page-cnt 600 \
  --funk-only 1 \
  --index-max 600000000 \
  --txns-max 1024 \
  --checkpt-funk /data/firedancer/ledger/mainnet-funk \
  --snapshot /data/firedancer/ledger/${SNAPSHOT:1}

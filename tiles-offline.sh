set -e

# git pull

EXTRAS=no-solana make -j fddev
make -j

sudo pkill -9 -f fddev || true
sudo ./build/native/gcc/bin/fddev configure fini all || true
sudo ./build/native/gcc/bin/fddev --config mainnet.toml configure fini all || true

# rm -f /data/arjain/snapshot-*
# wget --trust-server-names https://api.mainnet-beta.solana.com/snapshot.tar.bz2 -P /data/arjain

# rm -f /data/arjain/ledger-new/mainnet-funk
# rm -f /data/arjain/ledger-new/mainnet-blockstore
# sudo ./build/native/gcc/bin/fd_ledger \
# --cmd ingest \
# --page-cnt 32 \
# --funk-page-cnt 550 \
# --index-max 600000000 \
# --txns-max 1024 \
# --rocksdb /data/arjain/ledger-new/rocksdb \
# --checkpt-funk /data/arjain/ledger-new/mainnet-funk \
# --snapshot /data/arjain/ledger-new/snapshot-* \
# --incremental /data/arjain/ledger-new/incremental-* \
# --checkpt /data/arjain/ledger-new/mainnet-blockstore


# rm -f incremental-snapshot-*
# wget --trust-server-names https://api.mainnet-beta.solana.com/incremental-snapshot.tar.bz2

GOSSIP_PORT=$(shuf -i 8000-10000 -n 1)

echo "[gossip]
    port = $GOSSIP_PORT
[tiles]
    [tiles.gossip]
        entrypoints = [\"0.0.0.0\"]
        peer_ports = [7500]
        gossip_listen_port = $GOSSIP_PORT
    [tiles.repair]
        repair_intake_listen_port = $(shuf -i 8000-10000 -n 1)
        repair_serve_listen_port = $(shuf -i 8000-10000 -n 1)
    [tiles.replay]
        snapshot = \"wksp:/data/arjain/ledger-new/mainnet-funk\"
        incremental = \"incremental-snapshot-270030619-270044813-5wZmhnjq6pokrw2LQTnuwJi1nPkbR3NHxEvBMjao8mDv.tar.zst\"
        tpool_thread_count = 13
        funk_sz_gb = 550
        funk_txn_max = 1024
        funk_rec_max = 600000000
        capture = \"/data/arjain/tiles-offline.solcap\"
    [tiles.store_int]
        blockstore = \"/data/arjain/ledger-new/mainnet-blockstore\"
[consensus]
    expected_shred_version = 50093
[log]
  path = \"fddev.log\"
  level_stderr = \"NOTICE\"
[development]
    topology = \"firedancer\"
" > mainnet.toml

sudo gdb -ex=r --args ./build/native/gcc/bin/fddev --config mainnet.toml --no-sandbox --no-clone --no-solana-labs

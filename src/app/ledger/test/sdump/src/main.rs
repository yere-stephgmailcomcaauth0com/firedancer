use {
    std::env,
    std::path::Path,
    solana_ledger::{
        blockstore::Blockstore,
        blockstore_options::{
            AccessType, BlockstoreOptions, LedgerColumnOptions,
            ShredStorageType,
        }
    }
};

extern crate serde_json;

const DEFAULT_LEDGER_TOOL_ROCKS_FIFO_SHRED_STORAGE_SIZE_BYTES: u64 = std::u64::MAX;

fn main() {
    let args: Vec<String> = env::args().collect();
//    let ledger_path = Path::new("/home/jsiegel/repos/solana/test-ledger");
    let ledger_path = Path::new(&args[1]);

    let shred_storage_type = match ShredStorageType::from_ledger_path(
        &ledger_path,
        DEFAULT_LEDGER_TOOL_ROCKS_FIFO_SHRED_STORAGE_SIZE_BYTES,
    ) {
        Some(s) => s,
        None => {
//            error!("Shred storage type cannot be inferred, the default RocksLevel will be used");
            ShredStorageType::RocksLevel
        }
    };

    let db = Blockstore::open_with_options(
        ledger_path,
        BlockstoreOptions {
            access_type: AccessType::Primary,
            recovery_mode: None,
            enforce_ulimit_nofile: true,
            column_options: LedgerColumnOptions {
                shred_storage_type: shred_storage_type.clone(),
                ..LedgerColumnOptions::default()
            },
        }).unwrap();

    let ret = db.get_complete_block(args[2].parse().unwrap(), true).unwrap();
    println!("{}", serde_json::to_string(&ret).unwrap());
}

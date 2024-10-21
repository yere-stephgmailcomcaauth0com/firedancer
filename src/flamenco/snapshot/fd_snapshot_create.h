#ifndef HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h
#define HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h

/* fd_snapshot_create.h provides APIs for creating a Labs-compatible
   snapshot from a slot execution context. */

#include "../fd_flamenco_base.h"
#include "../runtime/context/fd_exec_slot_ctx.h"
#include <stdio.h>

struct fd_snapshot_create_private;
typedef struct fd_snapshot_create_private fd_snapshot_create_t;

FD_PROTOTYPES_BEGIN

/* fd_snapshot_create_{align,footprint} return required memory region
   parameters for the fd_snapshot_create_t object.

   worker_cnt is the number of workers for parallel snapshot create
   (treated as 1UL parallel mode not available). compress_lvl is the
   Zstandard compression level.  compress_bufsz is the in-memory buffer
   for writes (larger buffers results in less frequent but larger write
   ops).  funk_rec_cnt is the number of slots in the funk rec hashmap.
   batch_acc_cnt is the max number of accounts per account vec.

   Resulting footprint approximates

     O( funk_rec_cnt + (worker_cnt * (compress_lvl + compress_bufsz + batch_acc_cnt)) ) */

FD_FN_CONST ulong
fd_snapshot_create_align( void );

ulong
fd_snapshot_create_footprint( ulong worker_cnt,
                              int   compress_lvl,
                              ulong compress_bufsz,
                              ulong funk_rec_cnt,
                              ulong batch_acc_cnt );

/* fd_snapshot_create_new creates a new snapshot create object in the
   given mem region, which adheres to above alignment/footprint
   requirements.  Returns qualified handle to object given create object
   on success.  Serializes data from given slot context.  snap_path is
   the final snapshot path.  May create temporary files adject to
   snap_path.  {worker_cnt,compress_lvl,compress_bufsz,funk_rec_cnt,
   batch_acc_cnt} must match arguments to footprint when mem was
   created.  On failure, returns NULL. Reasons for failure include
   invalid memory region or invalid file descriptor.  Logs reasons for
   failure. */

fd_snapshot_create_t *
fd_snapshot_create_new( void *               mem,
                        fd_exec_slot_ctx_t * slot_ctx,
                        const char *         snap_path,
                        ulong                worker_cnt,
                        int                  compress_lvl,
                        ulong                compress_bufsz,
                        ulong                funk_rec_cnt,
                        ulong                batch_acc_cnt,
                        ulong                max_accv_sz,
                        fd_rng_t *           rng );

/* fd_snapshot_create_delete destroys the given snapshot create object
   and frees any resources.  Returns memory region and fd back to caller. */

void *
fd_snapshot_create_delete( fd_snapshot_create_t * create );

/* fd_snapshot_create exports the 'snapshot manifest' and a copy of all
   accounts from the slot ctx that the create object is attached to.
   Writes a .tar.zst stream out to the fd.  Returns 1 on success, and
   0 on failure.  Reason for failure is logged. */

int
fd_snapshot_create( fd_snapshot_create_t * create, 
                    fd_exec_slot_ctx_t *   slot_ctx );


static void FD_FN_UNUSED
fd_snapshot_create_populate_stakes( fd_exec_slot_ctx_t       * slot_ctx,
                                    fd_stakes_t              * old_stakes,
                                    fd_stakes_serializable_t * new_stakes ) {

  /* First populate the vote accounts using the vote accounts/stakes cache. 
     We can populate over all of the fields except we can't reserialize the
     vote account data. Instead we will copy over the raw contents of all of
     the vote accounts. */
  ulong vote_accounts_len = fd_vote_accounts_pair_t_map_size( old_stakes->vote_accounts.vote_accounts_pool, old_stakes->vote_accounts.vote_accounts_root );
  new_stakes->vote_accounts.vote_accounts_pool = fd_vote_accounts_pair_serializable_t_map_alloc( slot_ctx->valloc, fd_ulong_max(vote_accounts_len, 15000 ) );
  new_stakes->vote_accounts.vote_accounts_root = NULL;

  for( fd_vote_accounts_pair_t_mapnode_t * n = fd_vote_accounts_pair_t_map_minimum(
        old_stakes->vote_accounts.vote_accounts_pool,
        old_stakes->vote_accounts.vote_accounts_root );
       n;
       n = fd_vote_accounts_pair_t_map_successor( old_stakes->vote_accounts.vote_accounts_pool, n ) ) {
    
    fd_vote_accounts_pair_serializable_t_mapnode_t * new_node = fd_vote_accounts_pair_serializable_t_map_acquire( new_stakes->vote_accounts.vote_accounts_pool );
    new_node->elem.key   = n->elem.key;
    fd_memcpy( &new_node->elem.key, &n->elem.key, sizeof(fd_pubkey_t) );
    new_node->elem.stake = n->elem.stake;
    /* Now to populate the value, lookup the account using the acc mgr */
    FD_BORROWED_ACCOUNT_DECL( vote_acc );
    int err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, &n->elem.key, vote_acc );
    if( FD_UNLIKELY( err ) ) {
      FD_LOG_ERR(( "Failed to view vote account %s", FD_BASE58_ENC_32_ALLOCA(&n->elem.key) ));
    }
    new_node->elem.value.lamports   = vote_acc->const_meta->info.lamports;
    new_node->elem.value.data_len   = vote_acc->const_meta->dlen;
    new_node->elem.value.data       = fd_scratch_alloc( 16UL, vote_acc->const_meta->dlen );
    fd_memcpy( new_node->elem.value.data, vote_acc->const_data, vote_acc->const_meta->dlen );
    fd_memcpy( &new_node->elem.value.owner, &vote_acc->const_meta->info.owner, sizeof(fd_pubkey_t) );
    new_node->elem.value.executable = vote_acc->const_meta->info.executable;
    new_node->elem.value.rent_epoch = vote_acc->const_meta->info.rent_epoch;
    fd_vote_accounts_pair_serializable_t_map_insert( new_stakes->vote_accounts.vote_accounts_pool, &new_stakes->vote_accounts.vote_accounts_root, new_node );

  }

  /* Copy over the rest of the fields as they are the same. */
  new_stakes->stake_delegations_pool = old_stakes->stake_delegations_pool;
  new_stakes->stake_delegations_root = old_stakes->stake_delegations_root;
  new_stakes->unused                 = old_stakes->unused;
  new_stakes->epoch                  = old_stakes->epoch;
  fd_memcpy( &new_stakes->stake_history, &old_stakes->stake_history, fd_stake_history_size( &new_stakes->stake_history ) ); 
}

int FD_FN_UNUSED
fd_snapshot_create_manifest( fd_exec_slot_ctx_t * slot_ctx ) {
  FD_LOG_WARNING(("Creating manifest."));

  /* Populate the fields of the new manifest */
  fd_solana_manifest_serializable_t new_manifest = {0};
  //fd_solana_manifest_t              new_manifest = {0};
  fd_solana_manifest_t            * old_manifest = slot_ctx->solana_manifest;
  
  /* Copy in all the fields of the bank */

  /* blockhash_queue */
  fd_block_hash_vec_t blockhash_queue = {0};
  blockhash_queue.last_hash_index = slot_ctx->slot_bank.block_hash_queue.last_hash_index;
  blockhash_queue.last_hash = fd_scratch_alloc( 1UL, FD_HASH_FOOTPRINT );
  fd_memcpy( blockhash_queue.last_hash ,slot_ctx->slot_bank.block_hash_queue.last_hash, sizeof(fd_hash_t) );
  blockhash_queue.ages_len = 301UL; /* TODO: This should also be a constant */

  blockhash_queue.ages = fd_scratch_alloc( 1UL, blockhash_queue.ages_len * sizeof(fd_hash_hash_age_pair_t) );
  fd_block_hash_queue_t * queue = &slot_ctx->slot_bank.block_hash_queue;
  fd_hash_hash_age_pair_t_mapnode_t * nn;
  ulong blockhash_queue_idx = 0UL;
  for( fd_hash_hash_age_pair_t_mapnode_t * n = fd_hash_hash_age_pair_t_map_minimum( queue->ages_pool, queue->ages_root ); n; n = nn ) {
    nn = fd_hash_hash_age_pair_t_map_successor( queue->ages_pool, n );
    fd_hash_hash_age_pair_t elem = n->elem;
    fd_memcpy( &blockhash_queue.ages[ blockhash_queue_idx++ ], &elem, sizeof(fd_hash_hash_age_pair_t) );
  }

  blockhash_queue.max_age = 300UL; /* TODO: define this as a constant */

  new_manifest.bank.blockhash_queue       = blockhash_queue;
  new_manifest.bank.ancestors_len         = old_manifest->bank.ancestors_len;
  new_manifest.bank.ancestors             = old_manifest->bank.ancestors;
  new_manifest.bank.hash                  = old_manifest->bank.hash;
  new_manifest.bank.parent_hash           = old_manifest->bank.parent_hash;
  new_manifest.bank.parent_slot           = old_manifest->bank.parent_slot;
  new_manifest.bank.hard_forks            = old_manifest->bank.hard_forks;
  new_manifest.bank.transaction_count     = old_manifest->bank.transaction_count;
  new_manifest.bank.tick_height           = old_manifest->bank.tick_height;
  new_manifest.bank.signature_count       = old_manifest->bank.signature_count;
  new_manifest.bank.capitalization        = old_manifest->bank.capitalization;
  new_manifest.bank.max_tick_height       = old_manifest->bank.max_tick_height;
  new_manifest.bank.hashes_per_tick       = old_manifest->bank.hashes_per_tick;
  new_manifest.bank.ticks_per_slot        = old_manifest->bank.ticks_per_slot;
  new_manifest.bank.ns_per_slot           = old_manifest->bank.ns_per_slot;
  new_manifest.bank.genesis_creation_time = old_manifest->bank.genesis_creation_time;
  new_manifest.bank.slots_per_year        = old_manifest->bank.slots_per_year;
  new_manifest.bank.accounts_data_len     = old_manifest->bank.accounts_data_len;
  new_manifest.bank.slot                  = old_manifest->bank.slot;
  new_manifest.bank.epoch                 = old_manifest->bank.epoch;
  new_manifest.bank.block_height          = old_manifest->bank.block_height;
  new_manifest.bank.collector_id          = old_manifest->bank.collector_id;
  new_manifest.bank.collector_fees        = old_manifest->bank.collector_fees;
  new_manifest.bank.fee_calculator        = old_manifest->bank.fee_calculator;
  new_manifest.bank.fee_rate_governor     = old_manifest->bank.fee_rate_governor;
  new_manifest.bank.collected_rent        = old_manifest->bank.collected_rent;
  new_manifest.bank.rent_collector        = old_manifest->bank.rent_collector;
  new_manifest.bank.epoch_schedule        = old_manifest->bank.epoch_schedule;
  new_manifest.bank.inflation             = old_manifest->bank.inflation;
  //new_manifest.bank.stakes                = old_manifest->bank.stakes;
  new_manifest.bank.unused_accounts       = old_manifest->bank.unused_accounts;
  new_manifest.bank.epoch_stakes_len      = old_manifest->bank.epoch_stakes_len;
  new_manifest.bank.epoch_stakes          = old_manifest->bank.epoch_stakes;
  new_manifest.bank.is_delta              = old_manifest->bank.is_delta;

  /* Deserialized stakes cache is NOT equivalent to the one that we need to
     serialize because of the way vote accounts are stored */
  fd_snapshot_create_populate_stakes( slot_ctx, &old_manifest->bank.stakes, &new_manifest.bank.stakes );  

  /* Assign the other fields of the manifest to the serializable manifest */
  new_manifest.accounts_db                           = old_manifest->accounts_db;
  new_manifest.lamports_per_signature                = old_manifest->lamports_per_signature;
  new_manifest.bank_incremental_snapshot_persistence = old_manifest->bank_incremental_snapshot_persistence;
  new_manifest.epoch_account_hash                    = old_manifest->epoch_account_hash;
  new_manifest.versioned_epoch_stakes_len            = old_manifest->versioned_epoch_stakes_len;
  new_manifest.versioned_epoch_stakes                = old_manifest->versioned_epoch_stakes;

  // /* Encode and output the manifest to a file */
  ulong old_manifest_sz = fd_solana_manifest_size( old_manifest );
  ulong new_manifest_sz = fd_solana_manifest_serializable_size( &new_manifest ); 
  //ulong new_manifest_sz = fd_solana_manifest_size( &new_manifest );
  FD_LOG_WARNING(("OLD MANIFEST SIZE %lu", old_manifest_sz));
  FD_LOG_WARNING(("NEW MANIFEST SIZE %lu", new_manifest_sz));
  uchar * out_manifest = fd_scratch_alloc( 1UL, new_manifest_sz );
  fd_bincode_encode_ctx_t encode =
    { .data    = out_manifest,
      .dataend = out_manifest + new_manifest_sz + 1 };
  FD_TEST( 0==fd_solana_manifest_serializable_encode( &new_manifest, &encode ) );
  //FD_TEST( 0==fd_solana_manifest_encode( &new_manifest, &encode ) );

  FILE * file = fopen( "/data/ibhatt/manifest", "wb" );
  ulong  bytes_written= fwrite( out_manifest, 1, new_manifest_sz, file );
  if( bytes_written != new_manifest_sz ) {
    FD_LOG_ERR(("FAILED TO WRITE OUT"));
  }

  return 0;
}

int FD_FN_UNUSED
fd_snapshot_create_status_cache( void ) {
  return 0;
}

int FD_FN_UNUSED
fd_snapshot_create_acc_vecs( void ) {
  /* This should reference funk as well as the index and populate the actual
     append vecs */

  return 0;
}

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h */

// /*
//   // blockhash_queue 
//   // fd_block_hash_vec_t blockhash_queue = {0};
//   // blockhash_queue.last_hash_index = slot_ctx->slot_bank.block_hash_queue.last_hash_index;
//   // blockhash_queue.last_hash = fd_scratch_alloc( 1UL, FD_HASH_FOOTPRINT );
//   // fd_memcpy( blockhash_queue.last_hash ,slot_ctx->slot_bank.block_hash_queue.last_hash, sizeof(fd_hash_t) );
//   // blockhash_queue.ages_len = 301UL;  TODO: This should also be a constant 

//   // blockhash_queue.ages = fd_scratch_alloc( 1UL, blockhash_queue.ages_len * sizeof(fd_hash_hash_age_pair_t) );
//   // fd_block_hash_queue_t * queue = &slot_ctx->slot_bank.block_hash_queue;
//   // fd_hash_hash_age_pair_t_mapnode_t * nn;
//   // ulong blockhash_queue_idx = 0UL;
//   // for( fd_hash_hash_age_pair_t_mapnode_t * n = fd_hash_hash_age_pair_t_map_minimum( queue->ages_pool, queue->ages_root ); n; n = nn ) {
//   //   nn = fd_hash_hash_age_pair_t_map_successor( queue->ages_pool, n );
//   //   fd_hash_hash_age_pair_t elem = n->elem;
//   //   fd_memcpy( &blockhash_queue.ages[ blockhash_queue_idx++ ], &elem, sizeof(fd_hash_hash_age_pair_t) );
//   // }

//   // blockhash_queue.max_age = 300UL;  TODO: define this as a constant 
//   // //slot_ctx->solana_manifest->bank.blockhash_queue = blockhash_queue;

//   // // /* ancestors */
//   // FD_LOG_WARNING(("ANCESTOR LEN %lu", slot_ctx->solana_manifest->bank.ancestors_len));
//   // // for( ulong i=0UL; i<slot_ctx->solana_manifest->bank.ancestors_len; i++ ) {
//   // //   FD_LOG_WARNING(("ANCESTOR SLOT %lu val %lu", slot_ctx->solana_manifest->bank.ancestors[i].slot, slot_ctx->solana_manifest->bank.ancestors[i].val));
//   // // }
//   // // slot_ctx->solana_manifest->bank.ancestors_len = 0UL;
//   // // slot_ctx->solana_manifest->bank.ancestors     = NULL;

//   // /* hash */

//   // /* parent_hash */

#include "fd_snapshot_create.h"
#include "fd_snapshot.h"
#include "fd_snapshot_loader.h"
#include "fd_snapshot_restore.h"
#include "../runtime/fd_acc_mgr.h"
#include "../runtime/fd_hashes.h"
#include "../runtime/fd_runtime_init.h"
#include "../runtime/fd_system_ids.h"
#include "../runtime/context/fd_exec_epoch_ctx.h"
#include "../runtime/context/fd_exec_slot_ctx.h"
#include "../rewards/fd_rewards.h"

#include <assert.h>
#include <errno.h>

int
fd_snapshot_create_populate_acc_vec_idx( fd_exec_slot_ctx_t *                FD_FN_UNUSED slot_ctx,
                                         fd_solana_manifest_serializable_t * FD_FN_UNUSED manifest ) {

  fd_funk_t * funk = slot_ctx->acc_mgr->funk;

  void * mem                     = fd_valloc_malloc( slot_ctx->valloc, fd_acc_vecs_align(), fd_acc_vecs_footprint() );
  fd_acc_vecs_t * acc_vecs       = fd_acc_vecs_join( fd_acc_vecs_new( mem ) );
  fd_acc_vecs_t ** acc_vecs_iter = fd_scratch_alloc( 8UL, sizeof(fd_acc_vecs_t*) * 1<<20 );


  ulong id       = 10000000000UL;
  ulong start_id = id;
  ulong num_accs = 0UL;
  for (fd_funk_rec_t const *rec = fd_funk_txn_first_rec( funk, NULL ); NULL != rec; rec = fd_funk_txn_next_rec( funk, rec )) {


    uchar key[32];
    fd_base58_decode_32("JC3jTygxN7A3fEGPK9rMtN26f5HobTCydrkrQfG2B2M7", key);
    if( !memcmp( key, rec->pair.key, sizeof(fd_pubkey_t))) {
      FD_LOG_WARNING(("ASDF ASDF ASDF ASDF HERE"));
    }

    if( num_accs % 10000 == 0 ) {
      FD_LOG_WARNING(("ID %lu", id));
    }

    if( !fd_funk_key_is_acc( rec->pair.key ) ) {
      continue;
    }

    fd_pubkey_t const * pubkey = fd_type_pun_const( rec->pair.key[0].uc );
    (void)pubkey;

    uchar             const * raw      = fd_funk_val( rec, fd_funk_wksp( funk ) );
    fd_account_meta_t const * metadata = fd_type_pun_const( raw );

    if( !metadata ) {
      continue;
    }

    if( metadata->magic!=FD_ACCOUNT_META_MAGIC ) {
      continue;
    }

    uchar const * acc_data = raw + metadata->hlen;
    (void)acc_data;

    /* If it is the first record from a slot, create an append vec file,
       open a file descriptor */
    
    num_accs++;

    ulong record_slot = metadata->slot;

    fd_acc_vecs_t * acc_vec_key = fd_acc_vecs_query( acc_vecs, record_slot, NULL );
    if( !acc_vec_key ) {
      /* If entry for the slot does not exist, insert a key and assign it's id
         and slot. Create a corresponding filename that can be used to manage
         opening and closing the file descriptor. */
      acc_vec_key = fd_acc_vecs_insert( acc_vecs, record_slot );
      if( FD_UNLIKELY( !acc_vec_key ) ) {
        FD_LOG_ERR(( "Can't insert new acc vec" ));
      }
      fd_memset( acc_vec_key, 0UL, sizeof(fd_acc_vecs_t) );
      acc_vec_key->acc_vec.id = id;
      acc_vec_key->slot       = record_slot;

      /* Set the filename for the append vec. 
         TODO: the accounts directory should be parameterized. */
      char * filepath = acc_vec_key->filename;
      fd_memset( filepath, '\0', 256UL );
      fd_memcpy( filepath, "/data/ibhatt/dump/mainnet-254462437/own_snapshot/accounts/", 58UL );
      sprintf( filepath+58UL, "%lu.%lu", record_slot, id );

      /* Increment the id for the next append vec file */
      acc_vecs_iter[ id-start_id ] = acc_vec_key;
      id++;
    }

    // uchar key[32];
    // fd_base58_decode_32("JC3jTygxN7A3fEGPK9rMtN26f5HobTCydrkrQfG2B2M7", key);
    // if( !memcmp( key, pubkey, sizeof(fd_pubkey_t))) {
    //   FD_LOG_WARNING(("ASDF ASDF ASDF ASDF %lu", metadata->slot));
    // }

    /* The file size and number of keys should be updated.
       TODO: You technically don't need the count here. */
    acc_vec_key->count           += 1UL;
    acc_vec_key->acc_vec.file_sz += sizeof(fd_solana_account_hdr_t) + fd_ulong_align_up( metadata->dlen, FD_SNAPSHOT_ACC_ALIGN );

    char * file_path = acc_vec_key->filename;

    /* Open the file here. */
    int fd = open( file_path, O_CREAT | O_WRONLY | O_APPEND, 0644 );

    /* Write out the header. */
    fd_solana_account_hdr_t header = {0};
    /* Stored meta */
    header.meta.write_version_obsolete = 0UL;
    header.meta.data_len               = metadata->dlen;
    fd_memcpy( header.meta.pubkey, pubkey, sizeof(fd_pubkey_t) );
    /* Account Meta */
    header.info.lamports               = metadata->info.lamports;

    header.info.rent_epoch             = header.info.lamports ? metadata->info.rent_epoch : 0UL;
    fd_memcpy( header.info.owner, metadata->info.owner, sizeof(fd_pubkey_t) );
    header.info.executable             = metadata->info.executable;
    /* Hash */
    fd_memcpy( &header.hash, metadata->hash, sizeof(fd_hash_t) );

    ulong sz = 0UL;
    fd_io_write( fd, &header, sizeof(fd_solana_account_hdr_t), sizeof(fd_solana_account_hdr_t), &sz );
    if( FD_UNLIKELY( sz!=sizeof(fd_solana_account_hdr_t) ) ) {
      FD_LOG_ERR(( "Failed to write out the header" ));
    }
    fd_io_write( fd, acc_data, metadata->dlen, metadata->dlen, &sz );
    if( FD_UNLIKELY( sz!=metadata->dlen ) ) {
      FD_LOG_ERR(( "Failed to write out the account data" ));
    }
    ulong align_sz                       = fd_ulong_align_up( metadata->dlen, FD_SNAPSHOT_ACC_ALIGN ) - metadata->dlen;
    uchar padding[FD_SNAPSHOT_ACC_ALIGN] = {0};
    fd_io_write( fd, padding, align_sz, align_sz, &sz );
    if( FD_UNLIKELY( sz!=align_sz ) ) {
      FD_LOG_ERR(( "Failed to write out the padding" ));
    }

    close( fd );

  }

  /* At this point all of the append vec files have been written out. 
     We now need to populate the append vec index. */

  fd_bank_hash_info_t info = {0};

  manifest->accounts_db.storages_len                   = id-start_id;
  manifest->accounts_db.storages                       = fd_scratch_alloc( 8UL, manifest->accounts_db.storages_len * sizeof(fd_snapshot_slot_acc_vecs_t) );
  manifest->accounts_db.version                        = 1UL;
  manifest->accounts_db.slot                           = slot_ctx->slot_bank.slot - 1UL;
  manifest->accounts_db.bank_hash_info                 = info;
  manifest->accounts_db.historical_roots_len           = 0UL;
  manifest->accounts_db.historical_roots               = NULL;
  manifest->accounts_db.historical_roots_with_hash_len = 0UL;
  manifest->accounts_db.historical_roots_with_hash     = NULL;

  /* Populate the storages */
  for( ulong i=0UL; i<(id-start_id); i++ ) {
    fd_acc_vecs_t               * acc_vec = acc_vecs_iter[i];
    fd_snapshot_slot_acc_vecs_t * storage = &manifest->accounts_db.storages[i];
    
    storage->slot                    = acc_vec->slot;
    storage->account_vecs_len        = 1UL;
    storage->account_vecs            = fd_scratch_alloc( 8UL, sizeof(fd_snapshot_acc_vec_t) );
    storage->account_vecs[0].id      = acc_vec->acc_vec.id;
    storage->account_vecs[0].file_sz = acc_vec->acc_vec.file_sz;
    struct stat st;
    FD_TEST( 0 == stat(acc_vec->filename, &st) );
    FD_TEST((ulong)st.st_size == acc_vec->acc_vec.file_sz);
  }
  
  return 0;
}

void
fd_snapshot_create_serialiable_stakes( fd_exec_slot_ctx_t       * slot_ctx,
                                       fd_stakes_t              * old_stakes,
                                       fd_stakes_serializable_t * new_stakes ) {

/* The deserialized stakes cache that is used by the runtime can't be
     reserialized into the format that Agave uses. For every vote accounts
     in the stakes struct, the Firedancer client holds a decoded copy of the 
     vote state. However, this vote state can't be reserialized back into the 
     full vote account data. 
     
     This poses a problem in the Agave client client because upon boot, Agave
     verifies that for all of the vote accounts in the stakes struct, the data
     in the cache is the same as the data in the accounts db.
     
     The other problem is that the Firedancer stakes cache does not evict old
     entries and doesn't update delegations within the cache. The cache will
     just insert new pubkeys as stake accounts are created/delegated to. To
     make the cache conformant for the snapshot, old accounts should be removed
     from the snapshot and all of the delegations should be updated. */

  /* First populate the vote accounts using the vote accounts/stakes cache. 
     We can populate over all of the fields except we can't reserialize the
     vote account data. Instead we will copy over the raw contents of all of
     the vote accounts. */

  ulong vote_accounts_len                      = fd_vote_accounts_pair_t_map_size( old_stakes->vote_accounts.vote_accounts_pool, old_stakes->vote_accounts.vote_accounts_root );
  new_stakes->vote_accounts.vote_accounts_pool = fd_vote_accounts_pair_serializable_t_map_alloc( fd_scratch_virtual(), fd_ulong_max(vote_accounts_len, 15000 ) );
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
      FD_LOG_ERR(( "Failed to view vote account from stakes cache %s", FD_BASE58_ENC_32_ALLOCA(&n->elem.key) ));
    }

    new_node->elem.value.lamports   = vote_acc->const_meta->info.lamports;
    new_node->elem.value.data_len   = vote_acc->const_meta->dlen;
    new_node->elem.value.data       = fd_scratch_alloc( 8UL, vote_acc->const_meta->dlen );
    fd_memcpy( new_node->elem.value.data, vote_acc->const_data, vote_acc->const_meta->dlen );
    fd_memcpy( &new_node->elem.value.owner, &vote_acc->const_meta->info.owner, sizeof(fd_pubkey_t) );
    new_node->elem.value.executable = vote_acc->const_meta->info.executable;
    new_node->elem.value.rent_epoch = vote_acc->const_meta->info.rent_epoch;
    fd_vote_accounts_pair_serializable_t_map_insert( new_stakes->vote_accounts.vote_accounts_pool, &new_stakes->vote_accounts.vote_accounts_root, new_node );

  }

  /* Stale stake delegations should also be removed or updated in the cache. */

  FD_BORROWED_ACCOUNT_DECL( stake_acc );
  fd_delegation_pair_t_mapnode_t * nn = NULL;
  for( fd_delegation_pair_t_mapnode_t * n = fd_delegation_pair_t_map_minimum(
      old_stakes->stake_delegations_pool,
      old_stakes->stake_delegations_root );
      n; n=nn ) {

    nn = fd_delegation_pair_t_map_successor( old_stakes->stake_delegations_pool, n );
    
    int err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, &n->elem.account, stake_acc );
    if( FD_UNLIKELY( err ) ) {
      /* If the stake account doesn't exist, the cache is stale and the entry
         just needs to be evicted. */
      fd_delegation_pair_t_map_remove( old_stakes->stake_delegations_pool, &old_stakes->stake_delegations_root, n );
      fd_delegation_pair_t_map_release( old_stakes->stake_delegations_pool, n );
    } else {
      /* Otherwise, just update the delegation in case it is stale. */
      fd_bincode_decode_ctx_t ctx = {
        .data    = stake_acc->const_data,
        .dataend = stake_acc->const_data + stake_acc->const_meta->dlen,
        .valloc  = fd_scratch_virtual()
      };
      fd_stake_state_v2_t stake_state = {0};
      err = fd_stake_state_v2_decode( &stake_state, &ctx );
      if( FD_UNLIKELY( err ) ) {
        FD_LOG_ERR(( "Failed to decode stake state" ));
      }
      n->elem.delegation = stake_state.inner.stake.stake.delegation;
    }
  }

  /* Copy over the rest of the fields as they are the same. */
  new_stakes->stake_delegations_pool = old_stakes->stake_delegations_pool;
  new_stakes->stake_delegations_root = old_stakes->stake_delegations_root;
  new_stakes->unused                 = old_stakes->unused;
  new_stakes->epoch                  = old_stakes->epoch;
  new_stakes->stake_history          = old_stakes->stake_history;
}

int
fd_snapshot_create_populate_bank( fd_exec_slot_ctx_t * slot_ctx, fd_serializable_versioned_bank_t * bank ) {

  fd_epoch_bank_t * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );

  /* The blockhash queue has to be copied over along with all of its entries. */
  bank->blockhash_queue.last_hash_index = slot_ctx->slot_bank.block_hash_queue.last_hash_index;
  bank->blockhash_queue.last_hash       = fd_scratch_alloc( FD_HASH_ALIGN, FD_HASH_FOOTPRINT );
  fd_memcpy( bank->blockhash_queue.last_hash, slot_ctx->slot_bank.block_hash_queue.last_hash, sizeof(fd_hash_t) );

  bank->blockhash_queue.ages_len = fd_hash_hash_age_pair_t_map_size( slot_ctx->slot_bank.block_hash_queue.ages_pool, slot_ctx->slot_bank.block_hash_queue.ages_root);
  bank->blockhash_queue.ages     = fd_scratch_alloc( FD_HASH_HASH_AGE_PAIR_ALIGN, bank->blockhash_queue.ages_len * sizeof(fd_hash_hash_age_pair_t) );
  bank->blockhash_queue.max_age  = FD_BLOCKHASH_QUEUE_SIZE;

  fd_block_hash_queue_t * queue = &slot_ctx->slot_bank.block_hash_queue;
  fd_hash_hash_age_pair_t_mapnode_t * nn;
  ulong blockhash_queue_idx = 0UL;
  for( fd_hash_hash_age_pair_t_mapnode_t * n = fd_hash_hash_age_pair_t_map_minimum( queue->ages_pool, queue->ages_root ); n; n = nn ) {
    nn = fd_hash_hash_age_pair_t_map_successor( queue->ages_pool, n );
    fd_memcpy( &bank->blockhash_queue.ages[ blockhash_queue_idx++ ], &n->elem, sizeof(fd_hash_hash_age_pair_t) );
  }

  /* Ancestor can be omitted to boot off of for both clients */
  bank->ancestors_len                        = 0UL;
  bank->ancestors                            = NULL;

  bank->hash                                  = slot_ctx->slot_bank.banks_hash;
  bank->parent_hash                           = slot_ctx->prev_banks_hash;
  /* The slot needs to be decremented because the snapshot is created after
     a slot finishes executing and the slot value is incremented. 
     TODO: this will likely be different for the live case. */
  bank->parent_slot                           = slot_ctx->slot_bank.prev_slot - 1UL;

  /* Hard forks can be omitted as it is not needed to boot off of both clients */
  bank->hard_forks.hard_forks                 = NULL;
  bank->hard_forks.hard_forks_len             = 0UL;

  bank->transaction_count                     = slot_ctx->slot_bank.transaction_count;
  bank->tick_height                           = slot_ctx->tick_height;
  bank->signature_count                       = slot_ctx->parent_signature_cnt;
  bank->capitalization                        = slot_ctx->slot_bank.capitalization;
  bank->tick_height                           = slot_ctx->tick_height;
  bank->max_tick_height                       = slot_ctx->tick_height;
  bank->hashes_per_tick                       = &epoch_bank->hashes_per_tick;
  bank->ticks_per_slot                        = FD_TICKS_PER_SLOT;
  bank->ns_per_slot                           = epoch_bank->ns_per_slot;
  bank->genesis_creation_time                 = epoch_bank->genesis_creation_time;
  bank->slots_per_year                        = epoch_bank->slots_per_year;

  /* This value can be set to 0 because the Agave client recomputes this value
     and the firedancer client doesn't use it. */

  bank->accounts_data_len                     = 0UL;

  /* Need to subtract 1 for the same reason as the parent slot. */

  bank->slot                                  = slot_ctx->slot_bank.slot - 1UL;
  bank->epoch                                 = fd_slot_to_epoch( &epoch_bank->epoch_schedule, bank->slot, NULL );
  bank->block_height                          = slot_ctx->slot_bank.block_height;

  /* Value can be left as null for both clients */

  fd_memset( &bank->collector_id, 0, sizeof(fd_pubkey_t) );

  bank->collector_fees                        = slot_ctx->slot_bank.collected_execution_fees + slot_ctx->slot_bank.collected_priority_fees;
  bank->fee_calculator.lamports_per_signature = slot_ctx->slot_bank.lamports_per_signature;
  bank->fee_rate_governor                     = slot_ctx->slot_bank.fee_rate_governor;
  bank->collected_rent                        = slot_ctx->slot_bank.collected_rent;

  /* TODO: This needs more thorough testing on testnet/devnet wrt rent collection */
  bank->rent_collector.epoch                  = bank->epoch;
  bank->rent_collector.epoch_schedule         = epoch_bank->rent_epoch_schedule;
  bank->rent_collector.slots_per_year         = epoch_bank->slots_per_year;
  bank->rent_collector.rent                   = epoch_bank->rent;

  bank->epoch_schedule                        = epoch_bank->epoch_schedule;
  bank->inflation                             = epoch_bank->inflation;
  
  /* This can be left as null for both clients */
  fd_memset( &bank->unused_accounts, 0, sizeof(fd_unused_accounts_t) );

  /* We need to copy over the stakes for two epochs despite the Agave client
     providing the stakes for 6 epochs. These stakes need to be copied over
     because of the fact that the leader schedule computation uses the two
     previous epoch stakes. */
  fd_epoch_epoch_stakes_pair_t * relevant_epoch_stakes = fd_scratch_alloc( FD_EPOCH_EPOCH_STAKES_PAIR_ALIGN, 2UL * sizeof(fd_epoch_epoch_stakes_pair_t) );
  fd_memset( &relevant_epoch_stakes[0], 0UL, sizeof(fd_epoch_epoch_stakes_pair_t) );
  fd_memset( &relevant_epoch_stakes[1], 0UL, sizeof(fd_epoch_epoch_stakes_pair_t) );
  relevant_epoch_stakes[0].key                        = bank->epoch;
  relevant_epoch_stakes[0].value.stakes.vote_accounts = slot_ctx->slot_bank.epoch_stakes;
  relevant_epoch_stakes[1].key                        = bank->epoch+1UL;
  relevant_epoch_stakes[1].value.stakes.vote_accounts = epoch_bank->next_epoch_stakes;

  bank->epoch_stakes_len                      = 2UL;
  bank->epoch_stakes                          = relevant_epoch_stakes;
  bank->is_delta                              = 0;

  fd_snapshot_create_serialiable_stakes( slot_ctx, &epoch_bank->stakes, &bank->stakes );
  return 0;
}

int
fd_snapshot_create_new_snapshot( fd_exec_slot_ctx_t * slot_ctx, int FD_FN_UNUSED is_incremental ) {

  (void) is_incremental; /* TODO: this is unimplemented */

  FD_SCRATCH_SCOPE_BEGIN {
  /* Do a funk publish and whatnot */
  fd_funk_t *     funk = slot_ctx->acc_mgr->funk;
  fd_funk_txn_t * txn  = slot_ctx->funk_txn;
  fd_funk_start_write( funk );
  ulong publish_err = fd_funk_txn_publish( funk, txn, 1 );
  if( !publish_err ) {
    FD_LOG_ERR(("publish err"));
    return -1;
  }
  fd_funk_end_write( funk );


  /* Populate the fields of the new manifest */

  fd_solana_manifest_serializable_t new_manifest = {0};
  
  /* Copy in all the fields of the bank */
  fd_snapshot_create_populate_bank( slot_ctx, &new_manifest.bank );

  /* Populate the rest of the manifest, except for the append vec index */
  new_manifest.lamports_per_signature                = slot_ctx->slot_bank.lamports_per_signature;
  new_manifest.bank_incremental_snapshot_persistence = NULL;
  new_manifest.epoch_account_hash                    = &slot_ctx->slot_bank.epoch_account_hash;

  /* TODO: The versioned epoch stakes needs to be updated once these are being
     used in Agave. */
  new_manifest.versioned_epoch_stakes_len            = 0UL;
  new_manifest.versioned_epoch_stakes                = NULL;

  /* Create the append vecs and populate the index in the manifest. */

  fd_snapshot_create_populate_acc_vec_idx( slot_ctx, &new_manifest );
  fd_snapshot_hash( slot_ctx, NULL, &new_manifest.accounts_db.bank_hash_info.snapshot_hash, 0 );
  fd_memset( &new_manifest.accounts_db.bank_hash_info.stats, 0, sizeof(fd_bank_hash_stats_t) );

  /* TODO: Need to write out the snapshot hash to the name of the file as well. */

  FD_SCRATCH_SCOPE_BEGIN {

  fd_bank_slot_deltas_t slot_deltas_new = {0};
  fd_txncache_get_entries( slot_ctx->status_cache,
                            &slot_deltas_new );
  ulong bank_slot_deltas_sz = fd_bank_slot_deltas_size( &slot_deltas_new );
  uchar * out_status_cache = fd_scratch_alloc( FD_BANK_SLOT_DELTAS_ALIGN, bank_slot_deltas_sz );
  fd_bincode_encode_ctx_t encode_status_cache = {
    .data    = out_status_cache,
    .dataend = out_status_cache + bank_slot_deltas_sz,
  };
  if( FD_UNLIKELY( fd_bank_slot_deltas_encode( &slot_deltas_new, &encode_status_cache ) ) ) {
    FD_LOG_ERR(("Failed to encode the status cache"));
  }

  FILE * sc_file = fopen( "/data/ibhatt/dump/mainnet-254462437/own_snapshot/snapshots/status_cache", "wb" );
  ulong  bytes_written= fwrite( out_status_cache, 1, bank_slot_deltas_sz, sc_file );
  if( bytes_written != bank_slot_deltas_sz ) {
    FD_LOG_ERR(("Failed to write out the status cache"));
  }
  fclose(sc_file);

  } FD_SCRATCH_SCOPE_END;

  FD_LOG_WARNING(("ACCOUNTS DB HEADER %lu %lu %lu", new_manifest.accounts_db.storages_len, new_manifest.accounts_db.version, new_manifest.accounts_db.slot));

  // /* Encode and output the manifest to a file */
  ulong new_manifest_sz = fd_solana_manifest_serializable_size( &new_manifest ); 
  FD_LOG_WARNING(("NEW MANIFEST SIZE %lu", new_manifest_sz));
  uchar * out_manifest = fd_scratch_alloc( 1UL, new_manifest_sz );
  fd_bincode_encode_ctx_t encode =
    { .data    = out_manifest,
      .dataend = out_manifest + new_manifest_sz + 1 };
  FD_TEST( 0==fd_solana_manifest_serializable_encode( &new_manifest, &encode ) );

  FILE * file   = fopen( "/data/ibhatt/dump/mainnet-254462437/own_snapshot/snapshots/254462437/254462437", "wb" );
  ulong bytes_written = fwrite( out_manifest, 1, new_manifest_sz, file );
  if( FD_UNLIKELY( bytes_written!=new_manifest_sz ) ) {
    FD_LOG_ERR(("FAILED TO WRITE OUT"));
  }
  fclose(file);

  } FD_SCRATCH_SCOPE_END;

  return 0;
}

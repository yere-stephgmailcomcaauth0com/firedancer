#ifndef HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h
#define HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h

/* fd_snapshot_create.h provides APIs for creating a Labs-compatible
   snapshot from a slot execution context. */

#include "../fd_flamenco_base.h"
#include "fd_snapshot_base.h"
#include "../runtime/context/fd_exec_slot_ctx.h"
#include "../runtime/context/fd_exec_epoch_ctx.h"
#include "../runtime/sysvar/fd_sysvar_epoch_schedule.h"
#include "../runtime/fd_hashes.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>


struct fd_acc_vecs {
  ulong                 slot;
  ulong                 count;
  ulong                 next;
  fd_snapshot_acc_vec_t acc_vec;
  char                  filename[256];
  //int                   fd;
  /* These can be easily turned into snapshot_slot_acc_vecs */
};
typedef struct fd_acc_vecs fd_acc_vecs_t;

  /* clang-format off */
  #define MAP_NAME          fd_acc_vecs
  #define MAP_T             fd_acc_vecs_t
  #define MAP_KEY           slot
  #define MAP_MEMOIZE       0
  #define MAP_LG_SLOT_CNT   20
  #include "../../util/tmpl/fd_map.c"
  /* clang-format on */

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


static void
fd_snapshot_create_serialiable_stakes( fd_exec_slot_ctx_t       * slot_ctx,
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

    uchar key[32];
    fd_base58_decode_32( "FiijvR2ibXEHaFqB127CxrL3vSj19K2Kx1jf2RbK4BWS", key );
    if( !memcmp( &n->elem.key, key, sizeof(fd_pubkey_t) ) ) {
      FD_LOG_WARNING(("SLOT FOR FIJI %lu",vote_acc->const_meta->slot));
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

  FD_BORROWED_ACCOUNT_DECL( stake_acc );
  fd_delegation_pair_t_mapnode_t * nn = NULL;
  for( fd_delegation_pair_t_mapnode_t * n = fd_delegation_pair_t_map_minimum(
      old_stakes->stake_delegations_pool,
      old_stakes->stake_delegations_root );
      n; n=nn ) {

    nn = fd_delegation_pair_t_map_successor( old_stakes->stake_delegations_pool, n );
    
    int err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, &n->elem.account, stake_acc );
    if( FD_UNLIKELY( err ) ) {
      fd_delegation_pair_t_map_remove( old_stakes->stake_delegations_pool, &old_stakes->stake_delegations_root, n );
      fd_delegation_pair_t_map_release( old_stakes->stake_delegations_pool, n );
      FD_LOG_WARNING(("REMOVING %s", FD_BASE58_ENC_32_ALLOCA(&n->elem.account)));
    } else {

      uchar key[32];
      fd_base58_decode_32("FQgW8xrr6KDYivzfdeezaaVE3ws7xJBb7Fu27Zfk6c9n", key);

      if( !memcmp(key, &n->elem.account, sizeof(fd_pubkey_t)) ) {
        FD_LOG_WARNING(("SLOT FOR AGF %lu",n->elem.delegation.deactivation_epoch));
      }

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

      if( n->elem.delegation.deactivation_epoch == ULONG_MAX && stake_state.inner.stake.stake.delegation.deactivation_epoch != ULONG_MAX ) {
        FD_LOG_WARNING(("DELEGATION %lu", stake_state.inner.stake.stake.delegation.deactivation_epoch));
      }

      n->elem.delegation = stake_state.inner.stake.stake.delegation;
      if( !memcmp(key, &n->elem.account, sizeof(fd_pubkey_t)) ) {
        FD_LOG_WARNING(("SLOT FOR AGF 2 %lu",n->elem.delegation.deactivation_epoch));
      }

    }
  }
    

  /* Copy over the rest of the fields as they are the same. */
  new_stakes->stake_delegations_pool = old_stakes->stake_delegations_pool;
  new_stakes->stake_delegations_root = old_stakes->stake_delegations_root;
  new_stakes->unused                 = old_stakes->unused;
  new_stakes->epoch                  = old_stakes->epoch;
  new_stakes->stake_history          = old_stakes->stake_history;
}

int FD_FN_UNUSED
fd_snapshot_create_manifest( fd_exec_slot_ctx_t * slot_ctx ) {

  /* Do a funk */
  fd_funk_t *     funk = slot_ctx->acc_mgr->funk;
  fd_funk_txn_t * txn  = slot_ctx->funk_txn;
  fd_funk_start_write( funk );
  ulong publish_err = fd_funk_txn_publish( funk, txn, 1 );
  if( !publish_err ) {
    FD_LOG_ERR(("publish err"));
    return -1;
  }
  fd_funk_end_write( funk );

  // /****************** HACK TO GET THE ACC DB FROM THE NEWEST ***************/

  // FILE * ahead_manifest = fopen("/data/ibhatt/dump/mainnet-254462437/254462442", "rb");
  // FD_TEST(ahead_manifest);
  // fseek( ahead_manifest, 0, SEEK_END );
  // ulong file_sz = (ulong)ftell( ahead_manifest );
  // // rewind( ahead_manifest );
  // // uchar * buffer = fd_scratch_alloc( 8UL, file_sz );
  // // ulong fread_res = fread( buffer, 1, file_sz, ahead_manifest );
  // // FD_TEST(fread_res == file_sz);

  // // fclose( ahead_manifest );


  // fd_bincode_decode_ctx_t decode_ctx = {
  //   .data = buffer,
  //   .dataend = buffer + file_sz,
  //   .valloc = slot_ctx->valloc,
  // };
  // fd_solana_manifest_t newest_manifest = {0};
  // int decode_res = fd_solana_manifest_decode( &newest_manifest, &decode_ctx );
  // FD_TEST(!decode_res);


  fd_epoch_bank_t * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );

  /* Populate the fields of the new manifest */
  fd_solana_manifest_serializable_t new_manifest = {0};
  //fd_solana_manifest_t              new_manifest = {0};
  fd_solana_manifest_t            * old_manifest = slot_ctx->solana_manifest;
  
  /* Copy in all the fields of the bank */

  /* The blockhash queue can be populated */
  fd_block_hash_vec_t blockhash_queue = {0};
  blockhash_queue.last_hash_index = slot_ctx->slot_bank.block_hash_queue.last_hash_index;
  blockhash_queue.last_hash = fd_scratch_alloc( 1UL, FD_HASH_FOOTPRINT );
  fd_memcpy( blockhash_queue.last_hash ,slot_ctx->slot_bank.block_hash_queue.last_hash, sizeof(fd_hash_t) );
  blockhash_queue.ages_len = fd_hash_hash_age_pair_t_map_size( slot_ctx->slot_bank.block_hash_queue.ages_pool, slot_ctx->slot_bank.block_hash_queue.ages_root);

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
  new_manifest.bank.blockhash_queue       = blockhash_queue /* DONE! */;

  /* Ancestor can be omitted to boot off of for both clients */
  new_manifest.bank.ancestors_len         = 0UL; /* DONE!*/
  new_manifest.bank.ancestors             = NULL; /* DONE! */
  
  // new_manifest.bank.ancestors_len         = old_manifest->bank.ancestors_len;
  // new_manifest.bank.ancestors             = old_manifest->bank.ancestors;

  fd_pubkey_t pubkey_empty = {0};
  fd_unused_accounts_t unused_accounts = {0};

  // FD_LOG_WARNING(("FIREDANCER slot, epoch %lu %lu", slot_ctx->slot_bank.slot, fd_slot_to_epoch( &epoch_bank->epoch_schedule, slot_ctx->slot_bank.slot, NULL)));
  // FD_LOG_WARNING(("AGAVE slot, epoch %lu %lu", old_manifest->bank.slot, old_manifest->bank.epoch));
  // FD_LOG_WARNING(("BLOCK HEIGHT %lu %lu", slot_ctx->slot_bank.block_height, old_manifest->bank.block_height));
  // FD_LOG_WARNING(("ACCOUNTS DATA LEN %lu", old_manifest->bank.accounts_data_len));
  // FD_LOG_WARNING(("PREVIOUS SLOT %lu %lu", slot_ctx->slot_bank.prev_slot, old_manifest->bank.parent_slot));
  // FD_LOG_WARNING(("signature count %lu %lu %lu", slot_ctx->signature_cnt, slot_ctx->parent_signature_cnt, old_manifest->bank.signature_count));

  /* TODO:FIXME: Will likely need to adjust how we calculate the slot and prev slot. Maybe
     we have to get forks into play? We currently generate the snapshot before the start of
     a new slot. I think that this is mostly okay for now. See parent_signature_cnt too */

  FD_LOG_WARNING(("TICKS PER SLOT %lu %lu", epoch_bank->ticks_per_slot, old_manifest->bank.ticks_per_slot));
  FD_LOG_WARNING(("MAX BANK TICK HEIGHT %lu %lu", slot_ctx->slot_bank.max_tick_height, old_manifest->bank.max_tick_height));
  FD_LOG_WARNING(("MAX BANK TICK HEIGHT %lu %lu", slot_ctx->slot_bank.max_tick_height, old_manifest->bank.max_tick_height));
  FD_LOG_WARNING(("TICK HEIGHT %lu", old_manifest->bank.tick_height));
  FD_LOG_WARNING(("TICK HEIGHT %lu", old_manifest->bank.max_tick_height));

  new_manifest.bank.hash                  = slot_ctx->slot_bank.banks_hash; /* DONE! */
  new_manifest.bank.parent_hash           = slot_ctx->prev_banks_hash; /* DONE! */

  new_manifest.bank.parent_slot           = slot_ctx->slot_bank.prev_slot - 1UL; /* DONE! Need to subtract 1 here because of how agave does accounting. */

  new_manifest.bank.hard_forks.hard_forks     = NULL; /* DONE! */
  new_manifest.bank.hard_forks.hard_forks_len = 0UL;

  new_manifest.bank.transaction_count     = slot_ctx->slot_bank.transaction_count; /* DONE! */
  new_manifest.bank.tick_height           = slot_ctx->tick_height; /* DONE! */
  new_manifest.bank.signature_count       = slot_ctx->parent_signature_cnt; /* DONE! */

  new_manifest.bank.capitalization        = slot_ctx->slot_bank.capitalization; /* DONE! */

  new_manifest.bank.tick_height           = slot_ctx->tick_height; /* DONE! */
  new_manifest.bank.max_tick_height       = slot_ctx->tick_height; /* DONE! */

  new_manifest.bank.hashes_per_tick       = &epoch_bank->hashes_per_tick; /* DONE */

  new_manifest.bank.ticks_per_slot        = old_manifest->bank.ticks_per_slot;

  new_manifest.bank.ns_per_slot           = epoch_bank->ns_per_slot; /* DONE! */

  new_manifest.bank.genesis_creation_time = epoch_bank->genesis_creation_time; /* DONE! */

  new_manifest.bank.slots_per_year        = epoch_bank->slots_per_year; /* DONE! */

  new_manifest.bank.accounts_data_len     = 0UL; /* DONE! Agave recomputes this value from the accounts db that is loaded in anyway  */

  new_manifest.bank.slot                  = slot_ctx->slot_bank.slot - 1UL; /* DONE! Need to subtract 1 here because of how agave does accounting */

  new_manifest.bank.epoch                 = fd_slot_to_epoch( &epoch_bank->epoch_schedule, 
                                                              new_manifest.bank.slot,
                                                              NULL ); /* DONE! */

  new_manifest.bank.block_height          = slot_ctx->slot_bank.block_height; /* DONE! */

  new_manifest.bank.collector_id          = pubkey_empty; /* DONE! Can be omitted for both clients */

  new_manifest.bank.collector_fees        = slot_ctx->slot_bank.collected_execution_fees + 
                                            slot_ctx->slot_bank.collected_priority_fees; /* DONE! */

  new_manifest.bank.fee_calculator.lamports_per_signature = slot_ctx->slot_bank.lamports_per_signature; /* DONE! */

  new_manifest.bank.fee_rate_governor     = slot_ctx->slot_bank.fee_rate_governor; /* DONE! */
  new_manifest.bank.collected_rent        = slot_ctx->slot_bank.collected_rent; /* DONE! */

  /* TODO: This needs to get tested on testnet/devnet where rent is real */
  new_manifest.bank.rent_collector.epoch          = new_manifest.bank.epoch; /* DONE! */
  new_manifest.bank.rent_collector.epoch_schedule = epoch_bank->rent_epoch_schedule;
  new_manifest.bank.rent_collector.slots_per_year = epoch_bank->slots_per_year;
  new_manifest.bank.rent_collector.rent           = epoch_bank->rent;

  new_manifest.bank.epoch_schedule        = epoch_bank->epoch_schedule; /* DONE! */

  new_manifest.bank.inflation             = epoch_bank->inflation; /* DONE! */

  new_manifest.bank.unused_accounts       = unused_accounts; /* DONE! */

  FD_LOG_WARNING(("UNVERSIONED EPOCH STAKES %lu", old_manifest->bank.epoch_stakes_len));
  FD_LOG_WARNING(("new_manifest ticks %lu %lu", new_manifest.bank.tick_height, new_manifest.bank.max_tick_height));
  /* DONE! */
  /* We need to copy over the stakes for two epochs*/
  fd_epoch_epoch_stakes_pair_t relevant_epoch_stakes[2];
  fd_memset( &relevant_epoch_stakes[0], 0UL, sizeof(fd_epoch_epoch_stakes_pair_t) );
  relevant_epoch_stakes[0].key                        = new_manifest.bank.epoch;
  relevant_epoch_stakes[0].value.stakes.vote_accounts = slot_ctx->slot_bank.epoch_stakes;

  fd_memset( &relevant_epoch_stakes[1], 0UL, sizeof(fd_epoch_epoch_stakes_pair_t) );
  relevant_epoch_stakes[1].key                        = new_manifest.bank.epoch+1UL;
  relevant_epoch_stakes[1].value.stakes.vote_accounts = epoch_bank->next_epoch_stakes;

  new_manifest.bank.epoch_stakes_len                  = 2UL;
  new_manifest.bank.epoch_stakes                      = relevant_epoch_stakes;

  new_manifest.bank.is_delta                          = 0; /* DONE! */

  /* Deserialized stakes cache is NOT equivalent to the one that we need to
     serialize because of the way vote accounts are stored */
  fd_snapshot_create_serialiable_stakes( slot_ctx, &epoch_bank->stakes, &new_manifest.bank.stakes ); /* DONE! */

  /* AT THIS POINT THE BANK IS DONE *******************************************/
  /* Assign the other fields of the manifest to the serializable manifest */

  new_manifest.lamports_per_signature                = slot_ctx->slot_bank.lamports_per_signature; /* DONE! */

  new_manifest.bank_incremental_snapshot_persistence = NULL;
  new_manifest.epoch_account_hash                    = &slot_ctx->slot_bank.epoch_account_hash; /* DONE! */

  /* TODO: This needs to be properly populated instead of the epoch stakes in
     the bank when 2.1 gets activated on testnet. DONE! */
  new_manifest.versioned_epoch_stakes_len            = 0UL;
  new_manifest.versioned_epoch_stakes                = NULL;

  fd_hash_t acc_hash = {0};
  fd_snapshot_hash( slot_ctx, NULL, &acc_hash, 0);
  FD_LOG_WARNING(("ACCOUNTS DB HASH %s", FD_BASE58_ENC_32_ALLOCA(&acc_hash)));

  FD_LOG_WARNING(("bank hash info %s %s stats %lu %lu %lu %lu %lu", 
                  FD_BASE58_ENC_32_ALLOCA(&old_manifest->accounts_db.bank_hash_info.snapshot_hash), 
                  FD_BASE58_ENC_32_ALLOCA(&old_manifest->accounts_db.bank_hash_info.hash), 
                  old_manifest->accounts_db.bank_hash_info.stats.num_updated_accounts, 
                  old_manifest->accounts_db.bank_hash_info.stats.num_removed_accounts,
                  old_manifest->accounts_db.bank_hash_info.stats.num_lamports_stored,
                  old_manifest->accounts_db.bank_hash_info.stats.total_data_len,
                  old_manifest->accounts_db.bank_hash_info.stats.num_executable_accounts));

  fd_snapshot_create_populate_acc_vec_idx( slot_ctx, &new_manifest );
  //fd_hash_t hash = {0};
  fd_bank_hash_stats_t stats = {0};
  new_manifest.accounts_db.bank_hash_info.snapshot_hash = acc_hash;
  new_manifest.accounts_db.bank_hash_info.stats = stats;


  /* TODO: Need to write out the snapshot hash to the name of the file as well. */

  /* NOW POPulate the STATUS CACHE */
  //fd_bank_slot_deltas_t slot_deltas;


  FD_LOG_WARNING(("ACCOUNTS DB HEADER %lu %lu %lu", new_manifest.accounts_db.storages_len, new_manifest.accounts_db.version, new_manifest.accounts_db.slot));

  // /* Encode and output the manifest to a file */
  ulong old_manifest_sz = fd_solana_manifest_size( old_manifest );
  ulong new_manifest_sz = fd_solana_manifest_serializable_size( &new_manifest ); 
  FD_LOG_WARNING(("OLD MANIFEST SIZE %lu", old_manifest_sz));
  FD_LOG_WARNING(("NEW MANIFEST SIZE %lu", new_manifest_sz));
  uchar * out_manifest = fd_scratch_alloc( 1UL, new_manifest_sz );
  fd_bincode_encode_ctx_t encode =
    { .data    = out_manifest,
      .dataend = out_manifest + new_manifest_sz + 1 };
  FD_TEST( 0==fd_solana_manifest_serializable_encode( &new_manifest, &encode ) );

  FILE * file = fopen( "/data/ibhatt/dump/mainnet-254462437/own_snapshot/snapshots/254462437/254462437", "wb" );
  ulong  bytes_written= fwrite( out_manifest, 1, new_manifest_sz, file );
  if( bytes_written != new_manifest_sz ) {
    FD_LOG_ERR(("FAILED TO WRITE OUT"));
  }
  fclose(file);

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

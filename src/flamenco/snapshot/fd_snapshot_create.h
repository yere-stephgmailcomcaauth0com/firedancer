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


int FD_FN_UNUSED
fd_snapshot_create_manifest( fd_exec_slot_ctx_t * slot_ctx ) {
  /* 1. Define a manifest object
     2. Should populate fields from the slot and epoch context
     3. Should iterate through funk and populate an acc vec index in the
        accounts db object */

  if( slot_ctx->slot_bank.slot != 330914784 ) {
    return 0;
  }

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
  //slot_ctx->solana_manifest->bank.blockhash_queue = blockhash_queue;

  // /* ancestors */
  FD_LOG_WARNING(("ANCESTOR LEN %lu", slot_ctx->solana_manifest->bank.ancestors_len));
  // for( ulong i=0UL; i<slot_ctx->solana_manifest->bank.ancestors_len; i++ ) {
  //   FD_LOG_WARNING(("ANCESTOR SLOT %lu val %lu", slot_ctx->solana_manifest->bank.ancestors[i].slot, slot_ctx->solana_manifest->bank.ancestors[i].val));
  // }
  // slot_ctx->solana_manifest->bank.ancestors_len = 0UL;
  // slot_ctx->solana_manifest->bank.ancestors     = NULL;

  /* hash */

  /* parent_hash */


  /* Encode and output the manifest to a file */
  ulong   manifest_sz  = fd_solana_manifest_size( slot_ctx->solana_manifest );
  FD_LOG_WARNING(("MANIFEST SIZE %lu", manifest_sz));
  uchar * out_manifest = fd_scratch_alloc( 1UL, manifest_sz );
  fd_bincode_encode_ctx_t encode =
    { .data    = out_manifest,
      .dataend = out_manifest + manifest_sz + 1 };
  FD_TEST( 0==fd_solana_manifest_encode( slot_ctx->solana_manifest, &encode ) );

  FILE * file = fopen( "/data/ibhatt/manifest", "wb" );
  ulong  bytes_written= fwrite( out_manifest, 1, manifest_sz, file );
  if( bytes_written != manifest_sz ) {
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

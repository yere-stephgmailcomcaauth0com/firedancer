#ifndef HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h
#define HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h

/* fd_snapshot_create.h provides APIs for creating a Agave-compatible
   snapshot from a slot execution context. */

#include "fd_snapshot_base.h"
#include "../runtime/fd_runtime_init.h"
#include "../runtime/fd_txncache.h"
#include "../../util/archive/fd_tar.h"
#include "../types/fd_types.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define FD_BLOCKHASH_QUEUE_SIZE       (300UL)
#define FD_TICKS_PER_SLOT             (64UL)
/* This is the reasonably tight upper bound for the number of writable 
   accounts in a slot. This is because a block has a limit of 48 million
   compute units. Each writable account lock costs 300 CUs. That means there
   can be up to 48M/300 writable accounts in a block. */
#define FD_WRITABLE_ACCS_IN_SLOT      (160000UL)

#define FD_SNAPSHOT_DIR_MAX           (256UL)
#define FD_SNAPSHOT_VERSION_FILE      ("version")
#define FD_SNAPSHOT_VERSION           ("1.2.0")
#define FD_SNAPSHOT_VERSION_LEN       (5UL)
#define FD_SNAPSHOT_STATUS_CACHE_FILE ("snapshots/status_cache")

FD_PROTOTYPES_BEGIN

/* fd_snapshot_ctx_t holds various data structures needed for snapshot
   creation. It contains the snapshot slot, the snapshot directory,
   whether the snapshot is incremental, the tarball writer, the allocator,
   and holds the snapshot hash.
   
  NOTE: The snapshot service will currently not correctly free memory that is
        allocated unless a bump allocator like fd_scratch or fd_spad are used. */
struct fd_snapshot_ctx {
  ulong             snapshot_slot;
  char const *      snapshot_dir;
  uchar             is_incremental;
  fd_tar_writer_t * writer;
  fd_valloc_t       valloc;
  fd_hash_t         hash;
  fd_slot_bank_t    slot_bank;
  fd_epoch_bank_t   epoch_bank;
  fd_acc_mgr_t *    acc_mgr;
  fd_txncache_t *   status_cache;
};
typedef struct fd_snapshot_ctx fd_snapshot_ctx_t;

/* fd_snapshot_create_new_snapshot is responsible for creating the different
   structures used for snapshot generation and outputting them to a servable,
   compressed tarball. The main components of a Solana snapshot are as follows:

   1. Version - This is a file that contains the version of the snapshot.
   2. Manifest - The manifest contains data about the state of the network
                 as well as the index of the append vecs. 
      a. The bank. This is the equivalent of the firedancer slot/epoch context.
         This contains almost all of the state of the network that is not
         encapsulated in the accounts.
      b. Append vec index. This is a list of all of the append vecs that are
         used to store the accounts. This is a slot indexed file.
   3. Status cache - the status cache holds the transaction statuses for the 
      last 300 rooted slots. This is a nested data structure which is indexed
      by blockhash. See fd_txncache.h for more details on the status cache.
   4. Accounts directory - the accounts directory contains the state of all
      of the accounts and is a set of files described by <slot#.id#>. These
      are described by the append vec index in the manifest.
      
  The files are written out into a tar archive which is then zstd compressed. 
  
  This can produce either a full snapshot or an incremental snapshot depending
  on the value of is_incremental. An incremental snapshot will contain all of 
  the information described above, except it will only contain accounts that
  have been modified or deleted since the creation of the last incremental
  snapshot.

  TODO: Currently incremental snapshots are not supported. */

int
fd_snapshot_create_new_snapshot( fd_snapshot_ctx_t * snapshot_ctx );

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h */

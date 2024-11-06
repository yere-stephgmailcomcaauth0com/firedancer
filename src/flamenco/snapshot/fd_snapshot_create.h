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
#include "../../util/archive/fd_tar.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define FD_BLOCKHASH_QUEUE_SIZE       (300UL)
#define FD_TICKS_PER_SLOT             (64UL)

#define FD_SNAPSHOT_DIR_MAX           (256UL)
#define FD_SNAPSHOT_VERSION_FILE      ("version")
#define FD_SNAPSHOT_VERSION           ("1.2.0")
#define FD_SNAPSHOT_VERSION_LEN       (5UL)
#define FD_SNAPSHOT_STATUS_CACHE_FILE ("snapshots/status_cache")

FD_PROTOTYPES_BEGIN

struct fd_snapshot_ctx {
  ulong             snapshot_slot;
  char const *      snapshot_dir;
  int               is_incremental;
  fd_tar_writer_t * writer;
  fd_valloc_t       valloc;
  fd_hash_t         hash;
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
      of the accounts and is a set of files described by <slot#.id#>. 
      
  The files are written out into a tar archive which is then compressed with
  zstd.  */

int
fd_snapshot_create_new_snapshot( fd_snapshot_ctx_t * snapshot_ctx, fd_exec_slot_ctx_t * slot_ctx );

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h */

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

#define FD_BLOCKHASH_QUEUE_SIZE (300UL)
#define FD_TICKS_PER_SLOT       (64UL)

struct fd_snapshot_create_private;
typedef struct fd_snapshot_create_private fd_snapshot_create_t;

FD_PROTOTYPES_BEGIN

int
fd_snapshot_create_populate_bank( fd_exec_slot_ctx_t * slot_ctx, fd_serializable_versioned_bank_t * bank );

/* fd_snapshot_create_new_snapshot is responsible for creating the different
   structures used for snapshot generation and outputting them to a servable,
   compressed tarball. The main components of a Solana snapshot are as follows:
   1. The manifest - The manifest contains data about the state of the network
      as well as the index of the append vecs. 
      a. The bank. This is the equivalent of the firedancer slot/epoch context.
         This contains almost all of the state of the network that is not
         encapsulated in the accounts.
      b. Append vec index. This is a list of all of the append vecs that are
         used to store the accounts. This is a slot indexed file.
   2. Status cache - the status cache holds the transaction statuses for the 
      last 300 rooted slots. This is a nested data structure which is indexed
      by blockhash. See fd_txncache.h for more details on the status cache.
   3. Accounts directory - the accounts directory contains the state of all
      of the accounts and is a set of files described by <slot#.id#>. */
/* TODO: Add all of the options here. Perhaps a fd_snapshot_create_ctx_t. */
int
fd_snapshot_create_new_snapshot( fd_exec_slot_ctx_t * slot_ctx, int FD_FN_UNUSED is_incremental );

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_snapshot_fd_snapshot_create_h */

/* Gossip verify tile sits before the gossip (dedup?) tile to verify incoming
   gossip packets */
#define _GNU_SOURCE

#include "../../../../disco/tiles.h"

#include "../../../../disco/fd_disco.h"
#include "../../../../flamenco/gossip/fd_gossip.h"
#include "../../../../disco/keyguard/fd_keyload.h"
#include "../../../../disco/store/util.h"
#include "../../../../flamenco/runtime/fd_system_ids.h"
#include "../../../../util/fd_util.h"
#include "../../../../util/net/fd_eth.h"
#include "../../../../util/net/fd_ip4.h"
#include "../../../../util/net/fd_udp.h"
#include "../../../../util/net/fd_net_headers.h"

#define PACKET_DATA_SIZE 1232

struct fd_snapshot_tile_ctx {
  ulong        slot;
  char const * out_dir;
};
typedef struct fd_snapshot_tile_ctx fd_snapshot_tile_ctx_t;

FD_FN_CONST static inline ulong
scratch_align( void ) {
  return 128UL;
}


FD_FN_PURE static inline ulong
scratch_footprint( fd_topo_tile_t const * tile FD_PARAM_UNUSED ) {
  ulong l = FD_LAYOUT_INIT;
  l = FD_LAYOUT_APPEND( l, alignof(fd_snapshot_tile_ctx_t), sizeof(fd_snapshot_tile_ctx_t) );
  return FD_LAYOUT_FINI( l, scratch_align() );
}

static inline int
before_frag( fd_snapshot_tile_ctx_t * ctx    FD_PARAM_UNUSED,
             ulong                    in_idx FD_PARAM_UNUSED,
             ulong                    seq    FD_PARAM_UNUSED,
             ulong                    sig    FD_PARAM_UNUSED ) {

  FD_LOG_WARNING(("UNIMPLEMENTED"));
  return 0;
}

static void
during_frag( fd_snapshot_tile_ctx_t * ctx    FD_PARAM_UNUSED,
             ulong                    in_idx FD_PARAM_UNUSED,
             ulong                    seq    FD_PARAM_UNUSED,
             ulong                    sig    FD_PARAM_UNUSED,
             ulong                    chunk  FD_PARAM_UNUSED,
             ulong                    sz     FD_PARAM_UNUSED ) {
  
  FD_LOG_WARNING(("UNIMPLEMENTED"));
  return;
}

static void
after_frag( fd_snapshot_tile_ctx_t *  ctx    FD_PARAM_UNUSED,
            ulong                     in_idx FD_PARAM_UNUSED,
            ulong                     seq    FD_PARAM_UNUSED,
            ulong                     sig    FD_PARAM_UNUSED,
            ulong                     chunk  FD_PARAM_UNUSED,
            ulong                     sz     FD_PARAM_UNUSED,
            ulong                     tsorig FD_PARAM_UNUSED,
            fd_stem_context_t *       stem   FD_PARAM_UNUSED ) {
  FD_LOG_WARNING(("UNIMPLEMENTED"));
  return;
}

static void
privileged_init( fd_topo_t      * topo FD_PARAM_UNUSED,
                 fd_topo_tile_t * tile FD_PARAM_UNUSED ) {

  FD_LOG_WARNING(("UNIMPLEMENTED"));
  return;
}

static void
unprivileged_init( fd_topo_t      * topo FD_PARAM_UNUSED,
                   fd_topo_tile_t * tile FD_PARAM_UNUSED ) {
  FD_LOG_WARNING(("UNIMPLEMENTED"));
  return;
}

static ulong
populate_allowed_fds( fd_topo_t const *      topo        FD_PARAM_UNUSED,
                      fd_topo_tile_t const * tile        FD_PARAM_UNUSED,
                      ulong                  out_fds_cnt FD_PARAM_UNUSED,
                      int *                  out_fds     FD_PARAM_UNUSED ) {

  FD_LOG_WARNING(("UNIMPLEMENTED"));
  return 0;
}

#define STEM_BURST (1UL)

#define STEM_CALLBACK_CONTEXT_TYPE          fd_snapshot_tile_ctx_t
#define STEM_CALLBACK_CONTEXT_ALIGN alignof(fd_snapshot_tile_ctx_t)

#define STEM_CALLBACK_BEFORE_FRAG  before_frag
#define STEM_CALLBACK_DURING_FRAG  during_frag
#define STEM_CALLBACK_AFTER_FRAG   after_frag

#include "../../../../disco/stem/fd_stem.c"

fd_topo_run_tile_t fd_tile_gossip_verify = {
  .name                     = "gspvfy",
  .populate_allowed_fds     = populate_allowed_fds,
  .scratch_align            = scratch_align,
  .scratch_footprint        = scratch_footprint,
  .privileged_init          = privileged_init,
  .unprivileged_init        = unprivileged_init,
  .run                      = stem_run,
};

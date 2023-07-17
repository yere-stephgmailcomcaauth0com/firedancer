/****

build/linux/gcc/x86_64/bin/fd_rpc --wksp giant_wksp --gaddr 0xc7ce180

curl http://localhost:8899 -X POST -H 'content-type: application/json' --data '{"jsonrpc":"2.0", "id":1234, "method":"getAccountInfo", "params":["2cMzyuUE7VgDDVspERn8zo6dyrVsFgWi7G46QewbMEyc",{"encoding":"base58"}]}'

curl http://localhost:8899 -H 'content-type: application/json' --data '{"jsonrpc":"2.0", "id":1234, "method":"getBalance", "params":["7cVfgArCheMR6Cs4t6vz5rfnqd56vZq4ndaBrY5xkxXy"]}'

curl http://localhost:8899 -H 'content-type: application/json' --data '{"jsonrpc": "2.0","id":1,"method":"getBlock","params": [179248369,{"encoding": "json", "maxSupportedTransactionVersion":0, "transactionDetails":"full", "rewards":false}]}'

curl https://try-rpc.mainnet.solana.blockdaemon.tech -X POST -H 'content-type: application/json' --data '{"jsonrpc":"2.0", "id":1, "method":"getBlock", "params":[179248369,{"maxSupportedTransactionVers
ion":0}]}'

****/

#include "../../util/fd_util.h"
#include <signal.h>
#include <stdio.h>
#include <microhttpd.h>
#include "../../tango/webserver/fd_methods.h"
#include "../../tango/webserver/fd_webserver.h"
#include "../../funk/fd_funk.h"
#include "../../flamenco/types/fd_types.h"
#include "../../flamenco/runtime/fd_runtime.h"
#include "../../flamenco/runtime/fd_acc_mgr.h"
#include "../../ballet/base58/fd_base58.h"
#include "keywords.h"
#include "fd_block_to_json.h"

#define CRLF "\r\n"
#define MATCH_STRING(_text_,_text_sz_,_str_) (_text_sz_ == sizeof(_str_)-1 && memcmp(_text_, _str_, sizeof(_str_)-1) == 0)

static fd_funk_t* funk = NULL;
static fd_firedancer_banks_t bank;

// Implementation of the "getAccountInfo" method
int method_getAccountInfo(struct fd_web_replier* replier, struct json_values* values, long call_id) {
  // Path to argument
  static const uint PATH[3] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 0,
    (JSON_TOKEN_STRING<<16)
  };
  ulong arg_sz = 0;
  const void* arg = json_get_value(values, PATH, 3, &arg_sz);
  if (arg == NULL) {
    fd_web_replier_error(replier, "getAccountInfo requires a string as first parameter");
    return 0;
  }
  fd_pubkey_t acct;
  fd_base58_decode_32((const char *)arg, acct.uc);
  fd_funk_rec_key_t recid = fd_acc_mgr_key(&acct);
  fd_funk_rec_t const * rec = fd_funk_rec_query_global(funk, NULL, &recid);

  fd_textstream_t * ts = fd_web_replier_textstream(replier);
  if (rec == NULL) {
    fd_textstream_sprintf(ts, "{\"jsonrpc\":\"2.0\",\"result\":{\"context\":{\"apiVersion\":\"1.14.19\",\"slot\":%lu},\"value\":null},\"id\":%lu}" CRLF,
                          bank.slot, call_id);
    fd_web_replier_done(replier);
    return 0;
  }
  
  int err;
  void * val = fd_funk_val_cache(funk, rec, &err);
  if (val == NULL ) {
    fd_web_replier_error(replier, "failed to load account data for %s", (const char*)arg);
    return 0;
  }
  ulong val_sz = fd_funk_val_sz(rec);
  fd_account_meta_t * metadata = (fd_account_meta_t *)val;
  if (val_sz < metadata->hlen) {
    fd_web_replier_error(replier, "failed to load account data for %s", (const char*)arg);
    return 0;
  }
  val = (char*)val + metadata->hlen;
  val_sz = val_sz - metadata->hlen;
  if (val_sz > metadata->dlen)
    val_sz = metadata->dlen;
  
  static const uint PATH2[4] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 1,
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_ENCODING,
    (JSON_TOKEN_STRING<<16)
  };
  ulong enc_str_sz = 0;
  const void* enc_str = json_get_value(values, PATH2, 4, &enc_str_sz);
  enum { ENC_BASE58, ENC_BASE64, ENC_BASE64_ZSTD, ENC_JSON } enc;
  if (enc_str == NULL || MATCH_STRING(enc_str, enc_str_sz, "base58"))
    enc = ENC_BASE58;
  else if (MATCH_STRING(enc_str, enc_str_sz, "base64"))
    enc = ENC_BASE64;
  else if (MATCH_STRING(enc_str, enc_str_sz, "base64+zstd"))
    enc = ENC_BASE64_ZSTD;
  else if (MATCH_STRING(enc_str, enc_str_sz, "jsonParsed"))
    enc = ENC_JSON;
  else {
    fd_web_replier_error(replier, "invalid data encoding %s", (const char*)enc_str);
    return 0;
  }

  static const uint PATH3[4] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 2,
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_LENGTH,
    (JSON_TOKEN_INTEGER<<16)
  };
  static const uint PATH4[4] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 2,
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_OFFSET,
    (JSON_TOKEN_INTEGER<<16)
  };
  ulong len_sz = 0;
  const void* len_ptr = json_get_value(values, PATH3, 4, &len_sz);
  ulong off_sz = 0;
  const void* off_ptr = json_get_value(values, PATH4, 4, &off_sz);
  if (len_ptr && off_ptr) {
    if (enc == ENC_JSON) {
      fd_web_replier_error(replier, "cannot use jsonParsed encoding with slice");
      return 0;
    }
    long len = *(long*)len_ptr;
    long off = *(long*)off_ptr;
    if (off < 0 || (ulong)off >= val_sz) {
      val = NULL;
      val_sz = 0;
    } else {
      val = (char*)val + (ulong)off;
      val_sz = val_sz - (ulong)off;
    }
    if (len < 0) {
      val = NULL;
      val_sz = 0;
    } else if ((ulong)len < val_sz)
      val_sz = (ulong)len;
  }

  fd_textstream_sprintf(ts, "{\"jsonrpc\":\"2.0\",\"result\":{\"context\":{\"apiVersion\":\"1.14.19\",\"slot\":%lu},\"value\":{\"data\":\"",
                        bank.slot);

  if (val_sz) {
    switch (enc) {
    case ENC_BASE58:
      if (fd_textstream_encode_base58(ts, val, val_sz)) {
        fd_web_replier_error(replier, "failed to encode data in base58");
        return 0;
      }
      break;
    case ENC_BASE64:
      if (fd_textstream_encode_base64(ts, val, val_sz)) {
        fd_web_replier_error(replier, "failed to encode data in base64");
        return 0;
      }
      break;
    case ENC_BASE64_ZSTD:
      break;
    case ENC_JSON:
      break;
    }
  }
  
  char owner[50];
  fd_base58_encode_32((uchar*)metadata->info.owner, 0, owner);
  fd_textstream_sprintf(ts, "\",\"executable\":%s,\"lamports\":%lu,\"owner\":\"%s\",\"rentEpoch\":%lu}},\"id\":%lu}" CRLF,
                        (metadata->info.executable ? "true" : "false"),
                        metadata->info.lamports,
                        owner,
                        metadata->info.rent_epoch,
                        call_id);
  fd_web_replier_done(replier);
  return 0;
}

// Implementation of the "getBalance" method
int method_getBalance(struct fd_web_replier* replier, struct json_values* values, long call_id) {
  // Path to argument
  static const uint PATH[3] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 0,
    (JSON_TOKEN_STRING<<16)
  };
  ulong arg_sz = 0;
  const void* arg = json_get_value(values, PATH, 3, &arg_sz);
  if (arg == NULL) {
    fd_web_replier_error(replier, "getBalance requires a string as first parameter");
    return 0;
  }
  fd_pubkey_t acct;
  fd_base58_decode_32((const char *)arg, acct.uc);
  fd_funk_rec_key_t recid = fd_acc_mgr_key(&acct);
  fd_funk_rec_t const * rec = fd_funk_rec_query_global(funk, NULL, &recid);
  if (rec == NULL) {
    fd_web_replier_error(replier, "failed to load account data for %s", (const char*)arg);
    return 0;
  }
  int err;
  void * val = fd_funk_val_cache(funk, rec, &err);
  if (val == NULL ) {
    fd_web_replier_error(replier, "failed to load account data for %s", (const char*)arg);
    return 0;
  }
  fd_account_meta_t * metadata = (fd_account_meta_t *)val;
  fd_textstream_t * ts = fd_web_replier_textstream(replier);
  fd_textstream_sprintf(ts, "{\"jsonrpc\":\"2.0\",\"result\":{\"context\":{\"apiVersion\":\"1.14.19\",\"slot\":%lu},\"value\":%lu},\"id\":%lu}" CRLF,
                        bank.slot, metadata->info.lamports, call_id);
  fd_web_replier_done(replier);
  return 0;
}

// Implementation of the "getBlock" method
int method_getBlock(struct fd_web_replier* replier, struct json_values* values, long call_id) {
  static const uint PATH_SLOT[3] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 0,
    (JSON_TOKEN_INTEGER<<16)
  };
  static const uint PATH_ENCODING[4] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 1,
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_ENCODING,
    (JSON_TOKEN_STRING<<16)
  };
  static const uint PATH_MAXVERS[4] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 1,
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_MAXSUPPORTEDTRANSACTIONVERSION,
    (JSON_TOKEN_INTEGER<<16)
  };
  static const uint PATH_DETAIL[4] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 1,
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_TRANSACTIONDETAILS,
    (JSON_TOKEN_STRING<<16)
  };
  static const uint PATH_REWARDS[4] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_PARAMS,
    (JSON_TOKEN_LBRACKET<<16) | 1,
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_REWARDS,
    (JSON_TOKEN_BOOL<<16)
  };
  
  ulong slot_sz = 0;
  const void* slot = json_get_value(values, PATH_SLOT, 3, &slot_sz);
  if (slot == NULL) {
    fd_web_replier_error(replier, "getBlock requires a slot number as first parameter");
    return 0;
  }
  ulong slotn = (ulong)(*(long*)slot);

  ulong enc_str_sz = 0;
  const void* enc_str = json_get_value(values, PATH_ENCODING, 4, &enc_str_sz);
  enum fd_block_encoding enc;
  if (enc_str == NULL || MATCH_STRING(enc_str, enc_str_sz, "json"))
    enc = FD_BLOCK_ENC_JSON;
  else if (MATCH_STRING(enc_str, enc_str_sz, "base58"))
    enc = FD_BLOCK_ENC_BASE58;
  else if (MATCH_STRING(enc_str, enc_str_sz, "base64"))
    enc = FD_BLOCK_ENC_BASE64;
  else if (MATCH_STRING(enc_str, enc_str_sz, "jsonParsed"))
    enc = FD_BLOCK_ENC_JSON_PARSED;
  else {
    fd_web_replier_error(replier, "invalid data encoding %s", (const char*)enc_str);
    return 0;
  }

  ulong maxvers_sz = 0;
  const void* maxvers = json_get_value(values, PATH_MAXVERS, 4, &maxvers_sz);
  
  ulong det_str_sz = 0;
  const void* det_str = json_get_value(values, PATH_DETAIL, 4, &det_str_sz);
  enum fd_block_detail det;
  if (det_str == NULL || MATCH_STRING(det_str, det_str_sz, "full"))
    det = FD_BLOCK_DETAIL_FULL;
  else if (MATCH_STRING(det_str, det_str_sz, "accounts"))
    det = FD_BLOCK_DETAIL_ACCTS;
  else if (MATCH_STRING(det_str, det_str_sz, "signatures"))
    det = FD_BLOCK_DETAIL_SIGS;
  else if (MATCH_STRING(det_str, det_str_sz, "none"))
    det = FD_BLOCK_DETAIL_NONE;
  else {
    fd_web_replier_error(replier, "invalid block detail %s", (const char*)det_str);
    return 0;
  }
  
  ulong rewards_sz = 0;
  const void* rewards = json_get_value(values, PATH_REWARDS, 4, &rewards_sz);

  fd_funk_rec_key_t recid = fd_runtime_block_key(slotn);
  fd_funk_rec_t const * rec = fd_funk_rec_query_global(funk, NULL, &recid);
  if (rec == NULL) {
    fd_web_replier_error(replier, "failed to load block for slot %lu", slotn);
    return 0;
  }
  int err;
  void * val = fd_funk_val_cache(funk, rec, &err);
  if (val == NULL ) {
    fd_web_replier_error(replier, "failed to load block for slot %lu", slotn);
    return 0;
  }
  fd_textstream_t * ts = fd_web_replier_textstream(replier);
  if (fd_block_to_json(ts, call_id, val, fd_funk_val_sz(rec), enc,
                       (maxvers == NULL ? 0 : *(const long*)maxvers),
                       det,
                       (rewards == NULL ? 1 : *(const int*)rewards))) {
    fd_web_replier_error(replier, "failed to display block for slot %lu", slotn);
    return 0;
  }
  fd_web_replier_done(replier);
  return 0;
}

// Top level method dispatch function
void fd_webserver_method_generic(struct fd_web_replier* replier, struct json_values* values) {
  static const uint PATH[2] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_JSONRPC,
    (JSON_TOKEN_STRING<<16)
  };
  ulong arg_sz = 0;
  const void* arg = json_get_value(values, PATH, 2, &arg_sz);
  if (arg == NULL) {
    fd_web_replier_error(replier, "missing jsonrpc member");
    return;
  }
  if (!MATCH_STRING(arg, arg_sz, "2.0")) {
    fd_web_replier_error(replier, "jsonrpc value must be 2.0");
    return;
  }

  static const uint PATH3[2] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_ID,
    (JSON_TOKEN_INTEGER<<16)
  };
  arg_sz = 0;
  arg = json_get_value(values, PATH3, 2, &arg_sz);
  if (arg == NULL) {
    fd_web_replier_error(replier, "missing id member");
    return;
  }
  long call_id = *(long*)arg;

  static const uint PATH2[2] = {
    (JSON_TOKEN_LBRACE<<16) | KEYW_JSON_METHOD,
    (JSON_TOKEN_STRING<<16)
  };
  arg_sz = 0;
  arg = json_get_value(values, PATH2, 2, &arg_sz);
  if (arg == NULL) {
    fd_web_replier_error(replier, "missing method member");
    return;
  }
  long meth_id = fd_webserver_json_keyword((const char*)arg, arg_sz);

  switch (meth_id) {
  case KEYW_RPCMETHOD_GETACCOUNTINFO:
    if (!method_getAccountInfo(replier, values, call_id))
      return;
    break;
  case KEYW_RPCMETHOD_GETBALANCE:
    if (!method_getBalance(replier, values, call_id))
      return;
    break;
  case KEYW_RPCMETHOD_GETBLOCK:
    if (!method_getBlock(replier, values, call_id))
      return;
    break;
  default:
    fd_web_replier_error(replier, "unknown or unimplemented method %s", (const char*)arg);
    return;
  }

  /* Probably should make an error here */
  static const char* DOC=
"<html>" CRLF
"<head>" CRLF
"<title>OK</title>" CRLF
"</head>" CRLF
"<body>" CRLF
"</body>" CRLF
"</html>";
  fd_textstream_t * ts = fd_web_replier_textstream(replier);
  fd_textstream_append(ts, DOC, strlen(DOC));
  fd_web_replier_done(replier);
}
struct txn_map_key {
    fd_ed25519_sig_t v;
};
struct txn_map_elem {
    struct txn_map_key key;
    ulong slot;
    ulong blockoff;
    ulong next;
};
typedef struct txn_map_elem txn_map_elem_t;

ulong fd_ed25519_quickhash(const struct txn_map_key * key, ulong seed) {
  const ulong* x = (const ulong*)key;
  for (ulong i = 0; i < sizeof(struct txn_map_key)/sizeof(ulong); ++i)
    seed ^= x[i];
  return seed;
}

void fd_ed25519_quickcpy(struct txn_map_key * keydest, const struct txn_map_key * keysrc) {
  ulong* x = (ulong*)keydest;
  const ulong* y = (const ulong*)keysrc;
  for (ulong i = 0; i < sizeof(struct txn_map_key)/sizeof(ulong); ++i)
    x[i] = y[i];
}

int fd_ed25519_quickeq(const struct txn_map_key * key1, const struct txn_map_key * key2) {
  const ulong* x = (const ulong*)key1;
  const ulong* y = (const ulong*)key2;
  for (ulong i = 0; i < sizeof(struct txn_map_key)/sizeof(ulong); ++i)
    if (x[i] != y[i]) return 0;
  return 1;
}

#define MAP_KEY_T struct txn_map_key
#define MAP_NAME  txn_map_elem
#define MAP_T     txn_map_elem_t
#define MAP_KEY_HASH(key,seed) fd_ed25519_quickhash(key, seed)
#define MAP_KEY_COPY(keydest,keysrc) fd_ed25519_quickcpy(keydest, keysrc)
#define MAP_KEY_EQ(k0,k1) fd_ed25519_quickeq(k0, k1)
#include "../../util/tmpl/fd_map_giant.c"

static txn_map_elem_t * txn_map = NULL;

void prescan(void)  {
  fd_funk_rec_key_t key = fd_runtime_block_meta_key(ULONG_MAX);
  fd_funk_rec_t const * rec = fd_funk_rec_query( funk, NULL, &key );
  if (rec == NULL)
    FD_LOG_ERR(("missing meta record"));
  fd_slot_meta_meta_t mm;
  int err;
  const void * val = fd_funk_val_cache( funk, rec, &err );
  if (val == NULL)
    FD_LOG_ERR(("corrupt meta record"));
  fd_bincode_decode_ctx_t ctx2;
  ctx2.data    = val;
  ctx2.dataend = (uchar*)val + fd_funk_val_sz(rec);
  ctx2.valloc  = fd_libc_alloc_virtual();
  if ( fd_slot_meta_meta_decode( &mm, &ctx2 ) )
    FD_LOG_ERR(("fd_slot_meta_meta_decode failed"));
  FD_LOG_NOTICE(("scanning block %lu to %lu", mm.start_slot, mm.end_slot));

  /* Estimate an upper bound for the number of transactions based on the number of blocks */
  ulong key_max = 4000*(1 + mm.end_slot - mm.start_slot);
  void* mem = fd_valloc_malloc(fd_libc_alloc_virtual(), txn_map_elem_align(), txn_map_elem_footprint(key_max));
  txn_map = txn_map_elem_join(txn_map_elem_new(mem, key_max, 0));
  
  for (ulong slot = mm.start_slot; slot <= mm.end_slot; ++slot) {
    fd_funk_rec_key_t recid = fd_runtime_block_key(slot);
    rec = fd_funk_rec_query_global(funk, NULL, &recid);
    if (rec == NULL)
      continue;
    int err;
    const void * block = fd_funk_val_cache(funk, rec, &err);
    if (block == NULL) {
      FD_LOG_WARNING(("failed to load block for slot %lu", slot));
      continue;
    }
    ulong blocklen = fd_funk_val_sz( rec );

    /* Loop across batches */
    ulong blockoff = 0;
    while (blockoff < blocklen) {
      if ( blockoff + sizeof(ulong) > blocklen )
        FD_LOG_ERR(("premature end of block"));
      ulong mcount = *(const ulong *)((const uchar *)block + blockoff);
      blockoff += sizeof(ulong);

      /* Loop across microblocks */
      for (ulong mblk = 0; mblk < mcount; ++mblk) {
        if ( blockoff + sizeof(fd_microblock_hdr_t) > blocklen )
          FD_LOG_ERR(("premature end of block"));
        fd_microblock_hdr_t * hdr = (fd_microblock_hdr_t *)((const uchar *)block + blockoff);
        blockoff += sizeof(fd_microblock_hdr_t);

        /* Loop across transactions */
        for ( ulong txn_idx = 0; txn_idx < hdr->txn_cnt; txn_idx++ ) {
          fd_txn_xray_result_t xray;
          const uchar* raw = (const uchar *)block + blockoff;
          ulong pay_sz = fd_txn_xray(raw, blocklen - blockoff, &xray);
          if ( pay_sz == 0UL )
            FD_LOG_ERR(("failed to parse transaction %lu in microblock %lu in slot %lu", txn_idx, mblk, slot));
          
          struct txn_map_key const * sigs = (struct txn_map_key const *)((ulong)raw + (ulong)xray.signature_off);
          for ( ulong j = 0; j < xray.signature_cnt; j++ ) {
            txn_map_elem_t * elem = txn_map_elem_insert( txn_map, sigs+j );
            elem->slot = slot;
            elem->blockoff = blockoff;
          }
          
          blockoff += pay_sz;
        }
      }
    }

    if (blockoff != blocklen)
      FD_LOG_ERR(("garbage at end of block"));
  }
  FD_LOG_NOTICE(("scanned %lu transactions", txn_map_elem_key_cnt( txn_map )));
}

// SIGINT signal handler
volatile int stopflag = 0;
void stop(int sig) { (void)sig; stopflag = 1; }

int main(int argc, char** argv)
{
  fd_boot(&argc, &argv);

  const char* wkspname = fd_env_strip_cmdline_cstr(&argc, &argv, "--wksp", NULL, NULL);
  if (wkspname == NULL)
    FD_LOG_ERR(( "--wksp not specified" ));
  fd_wksp_t* wksp = fd_wksp_attach(wkspname);
  if (wksp == NULL)
    FD_LOG_ERR(( "failed to attach to workspace %s", wkspname ));
  
  const char* gaddr = fd_env_strip_cmdline_cstr(&argc, &argv, "--gaddr", NULL, NULL);
  if (gaddr == NULL)
    FD_LOG_ERR(( "--gaddr not specified" ));
  void* shmem;
  if (gaddr[0] == '0' && gaddr[1] == 'x')
    shmem = fd_wksp_laddr_fast( wksp, (ulong)strtol(gaddr+2, NULL, 16) );
  else
    shmem = fd_wksp_laddr_fast( wksp, (ulong)strtol(gaddr, NULL, 10) );
  funk = fd_funk_join(shmem);
  if (funk == NULL)
    FD_LOG_ERR(( "failed to join a funky" ));

  {
    fd_firedancer_banks_new(&bank);
    fd_funk_rec_key_t id = fd_runtime_banks_key();
    fd_funk_rec_t const * rec = fd_funk_rec_query_global(funk, NULL, &id);
    if ( rec == NULL )
      FD_LOG_ERR(("failed to read banks record"));
    int err;
    void * val = fd_funk_val_cache( funk, rec, &err );
    if (val == NULL )
      FD_LOG_ERR(("failed to read banks record"));
    fd_bincode_decode_ctx_t ctx;
    ctx.data = val;
    ctx.dataend = (uchar*)val + fd_funk_val_sz( rec );
    ctx.valloc = fd_libc_alloc_virtual();
    if ( fd_firedancer_banks_decode(&bank, &ctx ) )
      FD_LOG_ERR(("failed to read banks record"));
  }

  prescan();
  
  signal(SIGINT, stop);
  signal(SIGPIPE, SIG_IGN);

  // Get the gateway service port number 
  uint portno = fd_env_strip_cmdline_uint(&argc, &argv, "--port", "FD_FCGI_PORT", 8899U);
  fd_webserver_t ws;
  if (fd_webserver_start(portno, &ws))
    FD_LOG_ERR(("fd_webserver_start failed"));
  
  while (!stopflag) {
    sleep(1);
  }

  if (fd_webserver_stop(&ws))
    FD_LOG_ERR(("fd_webserver_stop failed"));

  {
    fd_bincode_destroy_ctx_t ctx;
    ctx.valloc = fd_libc_alloc_virtual();
    fd_firedancer_banks_destroy(&bank, &ctx);
  }

  fd_funk_leave( funk );

  fd_log_flush();
  fd_halt();
  return 0;
}

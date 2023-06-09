#include "../fd_tests.h"
int test_31(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 0;
  uchar disabled_features[] = { 2,76,118,26,24,61,83,56,117,106,108,80,82,33,15,103,29,90,125,55,62,116,75,124,113,78,105,128,120,121,114,77,110,79,30,27,98,109,87,112,122,127,92,123,126,111,89 };
  test.disable_feature = disabled_features;
  test.bt = "   2: solana_config_program::config_processor::tests::test_config_updates_requiring_config             at ./src/config_processor.rs:790:9   3: solana_config_program::config_processor::tests::test_config_updates_requiring_config::{{closure}}             at ./src/config_processor.rs:715:5   4: core::ops::function::FnOnce::call_once             at /rustc/0677edc86e342f333d4828b0ee1ef395a4e70fe5/library/core/src/ops/function.rs:227:5   5: core::ops::function::FnOnce::call_once             at /rustc/0677edc86e342f333d4828b0ee1ef395a4e70fe5/library/core/src/ops/function.rs:227:5";
  test.test_name = "config_processor::tests::test_config_updates_requiring_config";
  test.test_nonce  = 29;
  test.test_number = 31;
  if (fd_executor_test_suite_check_filter(suite, &test)) return -9999;
  ulong test_accs_len = 1;
  fd_executor_test_acc_t* test_accs = fd_alloca( 1UL, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );
  fd_memset( test_accs, 0, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );

  fd_executor_test_acc_t* test_acc = test_accs;
  fd_base58_decode_32( "4GEGFSiNnzwjeqJhmdhkxE5iG9LvtAYv5nFKd6v8qjEK",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Config1111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  test_acc->lamports        = 0UL;
  test_acc->result_lamports = 0UL;
  test_acc->executable      = 0;
  test_acc->rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_31_acc_0_data[] = { 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x30,0x79,0xc4,0xf3,0x78,0x78,0xa2,0x79,0xf6,0xe6,0x32,0x40,0xff,0x55,0x76,0x16,0xe6,0x7c,0x7c,0x2d,0xc8,0xac,0x51,0x88,0x5a,0x6e,0x08,0x0b,0xde,0xcc,0x83,0x3c,0x01,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_31_acc_0_data;
  test_acc->data_len        = 108UL;
  static uchar const fd_flamenco_native_prog_test_31_acc_0_post_data[] = { 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x30,0x79,0xc4,0xf3,0x78,0x78,0xa2,0x79,0xf6,0xe6,0x32,0x40,0xff,0x55,0x76,0x16,0xe6,0x7c,0x7c,0x2d,0xc8,0xac,0x51,0x88,0x5a,0x6e,0x08,0x0b,0xde,0xcc,0x83,0x3c,0x01,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_31_acc_0_post_data;
  test_acc->result_data_len = 108UL;
  test_acc++;
  fd_base58_decode_32( "Config1111111111111111111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_31_raw[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x02,0x30,0x79,0xc4,0xf3,0x78,0x78,0xa2,0x79,0xf6,0xe6,0x32,0x40,0xff,0x55,0x76,0x16,0xe6,0x7c,0x7c,0x2d,0xc8,0xac,0x51,0x88,0x5a,0x6e,0x08,0x0b,0xde,0xcc,0x83,0x3c,0x03,0x06,0x4a,0xa3,0x00,0x2f,0x74,0xdc,0xc8,0x6e,0x43,0x31,0x0f,0x0c,0x05,0x2a,0xf8,0xc5,0xda,0x27,0xf6,0x10,0x40,0x19,0xa3,0x23,0xef,0xa0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x4b,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x79,0xc4,0xf3,0x78,0x78,0xa2,0x79,0xf6,0xe6,0x32,0x40,0xff,0x55,0x76,0x16,0xe6,0x7c,0x7c,0x2d,0xc8,0xac,0x51,0x88,0x5a,0x6e,0x08,0x0b,0xde,0xcc,0x83,0x3c,0x01,0x2a,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test.raw_tx = fd_flamenco_native_prog_test_31_raw;
  test.raw_tx_len = 245UL;
  test.expected_result = -8;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}

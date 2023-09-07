#include "../fd_util.h"
#include "fd_toml.h"

FD_IMPORT_CSTR( test_case_0_input , "src/app/fdctl/config/default.toml" );


FD_IMPORT_CSTR( test_case_1_input ,    "src/util/toml/tests/valid/bool/bool.toml" );
FD_IMPORT_CSTR( test_case_1_expected , "src/util/toml/tests/valid/bool/bool.json" );


FD_IMPORT_CSTR( test_case_2_input ,    "src/util/toml/tests/valid/integer/integer.toml" );
FD_IMPORT_CSTR( test_case_2_expected , "src/util/toml/tests/valid/integer/integer.json" );

FD_IMPORT_CSTR( test_case_3_input ,    "src/util/toml/tests/valid/integer/underscore.toml" );
FD_IMPORT_CSTR( test_case_3_expected , "src/util/toml/tests/valid/integer/underscore.json" );

FD_IMPORT_CSTR( test_case_4_input ,    "src/util/toml/tests/valid/key/alphanum.toml" );
FD_IMPORT_CSTR( test_case_4_expected , "src/util/toml/tests/valid/key/alphanum.json" );

int
main( int     argc,
      char ** argv ) {

  fd_boot( &argc, &argv );

  fd_toml_table_t table_test_case_0;
  FD_LOG_NOTICE(( "TEST: %d", fd_toml_parse_document_from_string( test_case_0_input, test_case_0_input_sz, &table_test_case_0 ) ));

  fd_toml_table_t table_test_case_1;
  FD_LOG_NOTICE(( "TEST: %d", fd_toml_parse_document_from_string( test_case_1_input, test_case_1_input_sz, &table_test_case_1 ) ));

  char buf[65536];
  fd_toml_to_json( &table_test_case_1, buf );
  FD_LOG_NOTICE(( "A: |%s|", buf ));
  FD_LOG_NOTICE(( "B: |%s|", test_case_1_expected ));


  fd_toml_table_t table_test_case_2;
  FD_LOG_NOTICE(( "TEST: %d", fd_toml_parse_document_from_string( test_case_2_input, test_case_2_input_sz, &table_test_case_2 ) ));
  fd_toml_to_json( &table_test_case_2, buf );
  FD_LOG_NOTICE(( "A: |%s|", buf ));
  FD_LOG_NOTICE(( "B: |%s|", test_case_2_expected ));


  fd_toml_table_t table_test_case_3;
  FD_TEST( fd_toml_parse_document_from_string( test_case_3_input, test_case_3_input_sz, &table_test_case_3 )==0 );
  fd_toml_to_json( &table_test_case_3, buf );
  FD_LOG_NOTICE(( "A: |%s|", buf ));
  FD_LOG_NOTICE(( "B: |%s|", test_case_3_expected ));

  fd_toml_table_t table_test_case_4;
  FD_TEST( fd_toml_parse_document_from_string( test_case_4_input, test_case_4_input_sz, &table_test_case_4 )==0 );
  fd_toml_to_json( &table_test_case_4, buf );
  FD_LOG_NOTICE(( "A: |%s|", buf ));
  FD_LOG_NOTICE(( "B: |%s|", test_case_4_expected ));

  FD_LOG_NOTICE(( "pass" ));
  fd_halt();
  return 0;
}

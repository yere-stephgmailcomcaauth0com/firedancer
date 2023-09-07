#include "fd_toml.h"
#include <stdlib.h>

struct fd_toml_parser_context {
  const char *  content;
  ulong   cursor;
  ulong   len;
  ulong   line_num;
  
  fd_toml_table_t * root_table;
  fd_toml_table_t * current_table;
  fd_toml_table_t * last_table_entry;
  fd_toml_table_t * root_table_tail;

  char *  current_key_parts[1024];
  ulong   current_key_parts_len;
};
typedef struct fd_toml_parser_context fd_toml_parser_context_t;

uint
fd_toml_is_dec_digit( char c ) {
  return c >= '0' && c <= '9';
}

uint
fd_toml_is_minus_sign( char c ) {
  return c == '-';
}

uint
fd_toml_is_plus_sign( char c ) {
  return c == '+';
}

uint
fd_toml_is_hex_prefix( char c0, char c1 ) {
  return c0 == '0' && c1 == 'x';
}

uint
fd_toml_is_underscore( char c ) {
  return c == '_';
}

uint
fd_toml_is_quote( char c ) {
  return c == '"';
}

uint
fd_toml_is_whitespace( char c ) {
  return c == ' ' || c == '\t';
}

uint
fd_toml_is_newline( char c ) {
  return c == '\n';
}

uint
fd_toml_is_nul( char c ) {
  return c == '\0';
}

uint
fd_toml_is_dot( char c ) {
  return c == '.';
}

uint
fd_toml_is_num_sign( char c ) {
  return c == '#';
}

uint
fd_toml_is_letter( char c ) {
  return ( c >= 'A' && c <= 'Z' ) 
      || ( c >= 'a' && c <= 'z' );
}

uint
fd_toml_is_equals_sign( char c ) {
  return c == '=';
}

uint
fd_toml_is_left_square_bracket( char c ) {
  return c == '[';
}

uint
fd_toml_is_right_square_bracket( char c ) {
  return c == ']';
}

uint
fd_toml_is_boolean_t_or_f( char c ) {
  return c == 't' || c == 'f';
}

void
fd_toml_consume_char( fd_toml_parser_context_t * ctx ) {
  if( ctx->content[ctx->cursor] == '\n' ) {
    ctx->line_num++;
  }
  ctx->cursor++;
}

void
fd_toml_consume_whitespace( fd_toml_parser_context_t * ctx ) {
  char next_char0 = ctx->content[ctx->cursor];
  while( fd_toml_is_whitespace( next_char0 ) ) {
    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }
}

void
fd_toml_consume_whitespace_and_newlines( fd_toml_parser_context_t * ctx ) {
  char next_char0 = ctx->content[ctx->cursor];
  while( fd_toml_is_whitespace( next_char0 ) || fd_toml_is_newline( next_char0 ) ) {
    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }
}

int
fd_toml_parse_integer_decimal( fd_toml_parser_context_t * ctx, fd_toml_value_t * value ) {
  FD_LOG_NOTICE(( "parse: %6lu - integer_decimal", ctx->cursor ));
  
  long integer = 0;
  long sign = 1;

  char next_char0 = ctx->content[ctx->cursor];
  if( fd_toml_is_minus_sign( next_char0 ) ) {
    sign = -1;

    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  } else if( fd_toml_is_plus_sign( next_char0 ) ) {
    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }
  
  if( !fd_toml_is_dec_digit( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_DIGIT;
  }

  integer = (long)(next_char0 - '0');

  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];

  while( 1 ) {
    if( fd_toml_is_dec_digit( next_char0 ) ) {
      integer *= 10;
      integer += (long)(next_char0 - '0');
    } else if( fd_toml_is_nul( next_char0 ) ) {
      break;
    } else if( fd_toml_is_whitespace( next_char0 ) ) {
      break;
    } else if( fd_toml_is_newline( next_char0 ) ) {
      break;
    } else if( fd_toml_is_underscore( next_char0 ) ) {
      // no-op
    } else {
      return FD_TOML_PARSE_ERR_EXPECTED_DIGIT;
    }

    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }

  value->value_type = fd_toml_value_type_integer;
  value->value_content.integer_value = sign * integer;
  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_string( fd_toml_parser_context_t * ctx, fd_toml_value_t * value ) {
  FD_LOG_NOTICE(( "parse: %6lu - string", ctx->cursor ));

  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_quote( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_OPEN_QUOTE;
  }

  fd_toml_consume_char( ctx );
  ulong string_start_idx = ctx->cursor;
  next_char0 = ctx->content[ctx->cursor];

  while(1) {
    if( fd_toml_is_nul( next_char0 ) ) {
      return FD_TOML_PARSE_ERR_UNEXPECTED_EOF;
    } else if( fd_toml_is_newline( next_char0 ) ) {
      return FD_TOML_PARSE_ERR_UNEXPECTED_NEWLINE;
    } else if( fd_toml_is_quote( next_char0 ) ) {
      break;
    }
    // TODO: handle escaping
    
    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }
  
  ulong string_end_idx = ctx->cursor;
  fd_toml_consume_char( ctx );

  ulong string_len = string_end_idx - string_start_idx;
  char * string = (char *)malloc( sizeof( char ) * ( string_len + 1 ) );
  fd_memcpy( string, &ctx->content[string_start_idx], string_len );
  string[string_len] = '\0';

  value->value_type = fd_toml_value_type_string;
  value->value_content.string_value = string;
  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_boolean( fd_toml_parser_context_t * ctx, fd_toml_value_t * value ) {
  FD_LOG_NOTICE(( "parse: %6lu - boolean", ctx->cursor ));

  char next_char0 = ctx->content[ctx->cursor];
  char next_char1 = ((ctx->cursor + 1) > ctx->len) ? '\0' : ctx->content[ctx->cursor + 1];
  char next_char2 = ((ctx->cursor + 2) > ctx->len) ? '\0' : ctx->content[ctx->cursor + 2];
  char next_char3 = ((ctx->cursor + 3) > ctx->len) ? '\0' : ctx->content[ctx->cursor + 3];
  char next_char4 = ((ctx->cursor + 4) > ctx->len) ? '\0' : ctx->content[ctx->cursor + 4];

  uint boolean = 0;
  if( next_char0 == 'f' && next_char1 == 'a' && next_char2 == 'l' && next_char3 == 's' && next_char4 == 'e' ) {
    fd_toml_consume_char( ctx );
    fd_toml_consume_char( ctx );
    fd_toml_consume_char( ctx );
    fd_toml_consume_char( ctx );
    fd_toml_consume_char( ctx );

    boolean = 0;
  } else if(  next_char0 == 't' && next_char1 == 'r' && next_char2 == 'u' && next_char3 == 'e' ) {
    fd_toml_consume_char( ctx );
    fd_toml_consume_char( ctx );
    fd_toml_consume_char( ctx );
    fd_toml_consume_char( ctx );

    boolean = 1;
  } else {
    return FD_TOML_PARSE_ERR_EXPECTED_BOOLEAN;
  }

  value->value_type = fd_toml_value_type_boolean;
  value->value_content.boolean_value = boolean;
  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_array( fd_toml_parser_context_t * ctx, fd_toml_value_t * value ) {
  FD_LOG_NOTICE(( "parse: %6lu - array", ctx->cursor ));

  // char next_char0 = ctx->content[ctx->cursor];
  // FIXME: PARSE ARRAYS!
  (void)value;

  fd_toml_consume_char( ctx );
  fd_toml_consume_char( ctx );
  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_value( fd_toml_parser_context_t * ctx, fd_toml_value_t * value ) {
  FD_LOG_NOTICE(( "parse: %6lu - value", ctx->cursor ));

  char next_char0 = ctx->content[ctx->cursor];
  char next_char1 = ((ctx->cursor + 1) > ctx->len) ? '\0' : ctx->content[ctx->cursor + 1];
  (void) next_char1;

  int err;
  if( fd_toml_is_quote( next_char0 ) ) {
    if( ( err = fd_toml_parse_string( ctx, value ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  } else if( fd_toml_is_dec_digit( next_char0 ) 
      || fd_toml_is_minus_sign( next_char0 ) 
      || fd_toml_is_plus_sign( next_char0 ) ) {
    if( ( err = fd_toml_parse_integer_decimal( ctx, value ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  } else if( fd_toml_is_boolean_t_or_f( next_char0 ) ) {
    if( ( err = fd_toml_parse_boolean( ctx, value ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  } else if( fd_toml_is_left_square_bracket( next_char0 ) ) {
    if( ( err = fd_toml_parse_array( ctx, value ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  } else {
    return FD_TOML_PARSE_ERR_UNEXPECTED_INPUT;
  }

  return FD_TOML_PARSE_SUCCESS;
}



uint
fd_toml_is_key_part_char( char c ) {
  return fd_toml_is_letter( c ) 
      || fd_toml_is_dec_digit( c )
      || fd_toml_is_underscore( c )
      || fd_toml_is_minus_sign( c );
}

int
fd_toml_parse_key_part( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - key_part", ctx->cursor ));

  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_key_part_char( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_KEY;
  }

  ulong key_part_start_idx = ctx->cursor;
  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];

  while( 1 ) {
    if( fd_toml_is_key_part_char( next_char0 ) ) {
      // no-op
    } else if( fd_toml_is_dot( next_char0 ) ) {
      break;
    } else if( fd_toml_is_whitespace( next_char0 ) ) {
      break;
    } else if( fd_toml_is_equals_sign( next_char0 ) ) {
      break;
    } else {
      return FD_TOML_PARSE_ERR_EXPECTED_KEY;
    }
    
    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }

  ulong key_part_end_idx = ctx->cursor;
  ulong key_part_len = key_part_end_idx - key_part_start_idx;

  char * key_part = (char *)malloc( sizeof( char ) * ( key_part_len + 1 ) );
  fd_memcpy( key_part, &ctx->content[key_part_start_idx], key_part_len );
  key_part[key_part_len] = '\0';

  ctx->current_key_parts[ctx->current_key_parts_len++] = key_part;

  return FD_TOML_PARSE_SUCCESS;
}

int fd_toml_parse_key( fd_toml_parser_context_t * ctx );

int
fd_toml_parse_key_tail( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - key_tail", ctx->cursor ));

  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_dot( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_KEY;
  }

  fd_toml_consume_char( ctx );

  int err;
  if( ( err = fd_toml_parse_key( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }

  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_key( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - key", ctx->cursor ));
  char next_char0 = ctx->content[ctx->cursor];

  int err;
  if( ( err = fd_toml_parse_key_part( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }
  
  next_char0 = ctx->content[ctx->cursor];
  if( fd_toml_is_dot( next_char0 ) ) {
    if( ( err = fd_toml_parse_key_tail( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  } else if( fd_toml_is_whitespace( next_char0 ) ) {
    return FD_TOML_PARSE_SUCCESS;
  } else if( fd_toml_is_equals_sign( next_char0 ) ) {
    return FD_TOML_PARSE_SUCCESS;
  } else {
    return FD_TOML_PARSE_ERR_EXPECTED_KEY;
  }

  return FD_TOML_PARSE_ERR_EXPECTED_KEY;
}

int
fd_toml_add_key_value_pair( fd_toml_parser_context_t * ctx, fd_toml_value_t * value ) {
  FD_LOG_WARNING(( "ADD KVP1 "));
  fd_toml_table_t * new_table_entry = (fd_toml_table_t *)malloc( sizeof( fd_toml_table_t ) );
  fd_toml_table_t * prev_table_entry = ctx->last_table_entry;

  ulong i = 0;
  for( ; i < (ctx->current_key_parts_len - 1); i++ ) {
        FD_LOG_WARNING(( "ADD KVP4"));

    prev_table_entry->next = new_table_entry;
    new_table_entry->key_value_pair.key = ctx->current_key_parts[i];
    new_table_entry->key_value_pair.value.value_type = fd_toml_value_type_table;
    new_table_entry->key_value_pair.value.value_content.table_value = (fd_toml_table_t *)malloc( sizeof( fd_toml_table_t ) );

    new_table_entry = new_table_entry->key_value_pair.value.value_content.table_value;
  }
  FD_LOG_WARNING(( "ADD KVP2 "));

  prev_table_entry->next = new_table_entry;
  new_table_entry->key_value_pair.key = ctx->current_key_parts[i];
  new_table_entry->key_value_pair.value.value_type = value->value_type;
  new_table_entry->key_value_pair.value.value_content = value->value_content;

  ctx->last_table_entry = new_table_entry;

  new_table_entry = new_table_entry->key_value_pair.value.value_content.table_value;
    FD_LOG_WARNING(( "ADD KVP3 "));
  
  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_key_value_pair( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - key_value_pair", ctx->cursor ));
  int err;

  fd_toml_consume_whitespace( ctx );
  
  ctx->current_key_parts_len = 0;
  if( ( err = fd_toml_parse_key( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }
  for( ulong i = 0; i < ctx->current_key_parts_len; i++ ) {
    FD_LOG_NOTICE(( "key_part: %3lu - %s", i, ctx->current_key_parts[i] ));
  } 
  fd_toml_consume_whitespace( ctx );

  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_equals_sign( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_KV_PAIR;
  }
  fd_toml_consume_char( ctx );

  fd_toml_value_t * value = (fd_toml_value_t *)malloc( sizeof( fd_toml_value_t ) );
  fd_toml_consume_whitespace( ctx );
  if( ( err = fd_toml_parse_value( ctx, value ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }
  fd_toml_add_key_value_pair( ctx, value );

  fd_toml_consume_whitespace( ctx );

  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_table_key_part( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - table_key_part", ctx->cursor ));
  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_key_part_char( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_KEY;
  }

  ulong key_part_start_idx = ctx->cursor;
  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];

  while( 1 ) {
    if( fd_toml_is_key_part_char( next_char0 ) ) {
      // no-op
    } else if( fd_toml_is_dot( next_char0 ) ) {
      break;
    } else if( fd_toml_is_right_square_bracket( next_char0 ) ) {
      break;
    } else {
      return FD_TOML_PARSE_ERR_EXPECTED_KEY;
    }
    
    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }

  ulong key_part_end_idx = ctx->cursor;
  ulong key_part_len = key_part_end_idx - key_part_start_idx;

  char * key_part = (char *)malloc( sizeof( char ) * ( key_part_len + 1 ) );
  fd_memcpy( key_part, &ctx->content[key_part_start_idx], key_part_len );
  key_part[key_part_len] = '\0';
  ctx->current_key_parts[ctx->current_key_parts_len++] = key_part;

  return FD_TOML_PARSE_SUCCESS;
}

int fd_toml_parse_table_key( fd_toml_parser_context_t * ctx );

int
fd_toml_parse_table_key_tail( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - table_key_tail", ctx->cursor ));

  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_dot( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_KEY;
  }

  fd_toml_consume_char( ctx );

  int err;
  if( ( err = fd_toml_parse_table_key( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }

  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_set_current_table( fd_toml_parser_context_t * ctx ) {
  fd_toml_table_t * prev_table = ctx->root_table_tail;
  // FD_TEST( prev_table->next == NULL );

  fd_toml_table_t * table = (fd_toml_table_t *)malloc( sizeof( fd_toml_table_t ) );
  
  for( ulong i = 0; i < ctx->current_key_parts_len; i++ ) {
    FD_LOG_WARNING(( "SCT: %s", ctx->current_key_parts[i]));
    table->key_value_pair.key = ctx->current_key_parts[i];
    table->key_value_pair.value.value_type = fd_toml_value_type_table;
    fd_toml_table_t * next_table = (fd_toml_table_t *)malloc( sizeof( fd_toml_table_t ) );
    table->key_value_pair.value.value_content.table_value = next_table;
    prev_table->next = table;
    table->next = NULL;
    prev_table = table;
  }

  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_table_key( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - table_key", ctx->cursor ));
  char next_char0 = ctx->content[ctx->cursor];

  int err;
  if( ( err = fd_toml_parse_table_key_part( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }
  
  next_char0 = ctx->content[ctx->cursor];
  if( fd_toml_is_dot( next_char0 ) ) {
    if( ( err = fd_toml_parse_table_key_tail( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  }
  
  next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_right_square_bracket( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_KEY;
  }

  if ( !fd_toml_set_current_table( ctx ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_KEY;
  }

  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_comment( fd_toml_parser_context_t * ctx ) {
  FD_LOG_NOTICE(( "parse: %6lu - comment", ctx->cursor ));
  char next_char0 = ctx->content[ctx->cursor];
  
  if( !fd_toml_is_num_sign( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_COMMENT;
  }

  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];
  
  while( 1 ) {
    if( fd_toml_is_newline( next_char0 ) ) {
      break;
    } else if( fd_toml_is_nul( next_char0 ) ) {
      break;
    }

    fd_toml_consume_char( ctx );
    next_char0 = ctx->content[ctx->cursor];
  }
  
  return FD_TOML_PARSE_SUCCESS;
}

int
fd_toml_parse_array_of_tables( fd_toml_parser_context_t * ctx ) {
  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_left_square_bracket( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_TABLE;
  }

  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];

  if( !fd_toml_is_left_square_bracket( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_TABLE;
  }

  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];
  
  ctx->current_key_parts_len = 0;
  int err;
  if( ( err = fd_toml_parse_table_key( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }
  for( ulong i = 0; i < ctx->current_key_parts_len; i++ ) {
    FD_LOG_NOTICE(( "table_key_part: %3lu - %s", i, ctx->current_key_parts[i] ));
  }

  next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_left_square_bracket( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_TABLE;
  }

  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];

  if( !fd_toml_is_left_square_bracket( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_TABLE;
  }

  fd_toml_consume_char( ctx );
  return FD_TOML_PARSE_SUCCESS;
}

// char *
// fd_toml_to_json_indent( char * buf, const char * buf_end, ulong depth ) {
//   ulong indent = 2UL * depth;
//   // if( indent >= ( buf_end - buf ) ) {
//   //   return NULL;
//   // }

//   fd_memset( buf, ' ', indent );
//   buf += indent;
//   return buf;
// }



char *
fd_toml_to_json_integer_value( long integer_value, char * buf ) {
  return fd_cstr_append_printf(buf, "{\"type\":\"integer\",\"value\":\"%ld\"}", integer_value);
}

char *
fd_toml_to_json_float_value( double float_value, char * buf ) {
  return fd_cstr_append_printf(buf, "{\"type\":\"float\",\"value\":\"%f\"}", float_value);
}

char *
fd_toml_to_json_string_value( char * string_value, char * buf ) {
  return fd_cstr_append_printf(buf, "{\"type\":\"float\",\"value\":\"%s\"}", string_value);
}

char *
fd_toml_to_json_boolean_value( uint boolean_value, char * buf ) {
  if( boolean_value ) {
    return fd_cstr_append_printf(buf, "{\"type\":\"bool\",\"value\":\"true\"}");
  } else {
    return fd_cstr_append_printf(buf, "{\"type\":\"bool\",\"value\":\"false\"}");
  }
}

char *
fd_toml_to_json_value( fd_toml_value_t const * value, char * buf ) {
  switch( value->value_type ) {
    case fd_toml_value_type_boolean:
      return fd_toml_to_json_boolean_value( value->value_content.boolean_value, buf );
    case fd_toml_value_type_integer:
      return fd_toml_to_json_integer_value( value->value_content.integer_value, buf );
    case fd_toml_value_type_string:
      return fd_toml_to_json_string_value( value->value_content.string_value, buf );
    default:
      return fd_cstr_append_printf( buf, "unknown value type" );
  }
}

char *
fd_toml_to_json_key_value_pair( fd_toml_key_value_pair_t const * key_value_pair, char * buf ) {
  buf = fd_cstr_append_printf( buf, "\"%s\":", key_value_pair->key);
  buf = fd_toml_to_json_value( &key_value_pair->value, buf );
  return buf;
}

char *
fd_toml_to_json_table( fd_toml_table_t const * table, char * buf ) {
  buf = fd_cstr_append_printf( buf, "{" );
  while( table ) {
    buf = fd_toml_to_json_key_value_pair( &table->key_value_pair, buf );

    if( table->next ) {
      buf = fd_cstr_append_printf( buf, "," );
    }
    table = table->next;
  }
  buf = fd_cstr_append_printf( buf, "}" );
  return buf;
}



char *
fd_toml_to_json( fd_toml_table_t const * table, char * buf ) {  
  return fd_toml_to_json_table( table->next, buf );
}


int
fd_toml_parse_table( fd_toml_parser_context_t * ctx ) {
  char next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_left_square_bracket( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_TABLE;
  }

  fd_toml_consume_char( ctx );
  next_char0 = ctx->content[ctx->cursor];
  
  ctx->current_key_parts_len = 0;
  int err;
  if( ( err = fd_toml_parse_table_key( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
    return err;
  }
  for( ulong i = 0; i < ctx->current_key_parts_len; i++ ) {
    FD_LOG_NOTICE(( "table_key_part: %3lu - %s", i, ctx->current_key_parts[i] ));
  }

  next_char0 = ctx->content[ctx->cursor];
  if( !fd_toml_is_right_square_bracket( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_TABLE;
  }

  fd_toml_consume_char( ctx );
  return FD_TOML_PARSE_SUCCESS;
}

int 
fd_toml_parse_document( fd_toml_parser_context_t * ctx, 
                        fd_toml_table_t *          table ) {
  FD_LOG_NOTICE(( "parse: %6lu - document", ctx->cursor ));
  fd_toml_consume_whitespace_and_newlines( ctx );
  char next_char0 = ctx->content[ctx->cursor];
  char next_char1 = ((ctx->cursor + 1) > ctx->len) ? '\0' : ctx->content[ctx->cursor + 1];
  
  int err;
  if( fd_toml_is_nul( next_char0 ) ) {
    return FD_TOML_PARSE_SUCCESS;
  } else if( fd_toml_is_key_part_char( next_char0 ) ) {
    if( ( err = fd_toml_parse_key_value_pair( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  } else if( fd_toml_is_num_sign( next_char0 ) ) {
    if( ( err = fd_toml_parse_comment( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
      return err;
    }
  } else if( fd_toml_is_left_square_bracket( next_char0 ) ) {
    if ( fd_toml_is_left_square_bracket( next_char1 ) ) {
      if( ( err = fd_toml_parse_array_of_tables( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
        return err;
      }
    } else if ( fd_toml_is_key_part_char( next_char1 ) ) {
        if( ( err = fd_toml_parse_table( ctx ) ) != FD_TOML_PARSE_SUCCESS ) {
        return err;
      }
    }
  } else {
    return FD_TOML_PARSE_ERR_UNEXPECTED_INPUT;
  }
  
  fd_toml_consume_whitespace( ctx );

  next_char0 = ctx->content[ctx->cursor];
  if( fd_toml_is_nul( next_char0 ) ) {
    return FD_TOML_PARSE_SUCCESS;
  } else if( !fd_toml_is_newline( next_char0 ) ) {
    return FD_TOML_PARSE_ERR_EXPECTED_NEWLINE;
  }

  fd_toml_consume_char( ctx );

  return fd_toml_parse_document( ctx, table );
}

int
fd_toml_parse_document_from_string( const char *      content,
                                    ulong             content_len,
                                    fd_toml_table_t * table ) {
  fd_toml_parser_context_t ctx = {
    .content = content,
    .cursor = 0,
    .len = content_len,
    .line_num = 1,
    .root_table = table,
    .current_table = table,
    .last_table_entry = table,
    .root_table_tail = table,
  };

  int err = fd_toml_parse_document( &ctx, table );
  FD_LOG_NOTICE(("parsing done - err: %d, cursor: %lu, line: %lu, char: |%c|", err, ctx.cursor, ctx.line_num, ctx.content[ctx.cursor]));
  return err;
}

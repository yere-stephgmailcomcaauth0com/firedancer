#include "fd_ristretto255_ge.h"

static uchar const bad_encodings[][32] = {
  /* Non-canonical field encodings */
  "\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f",
  "\xf3\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f",
  "\xed\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f",
  /* Negative field elements */
  "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
  "\x01\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f",
  "\xed\x57\xff\xd8\xc9\x14\xfb\x20\x14\x71\xd1\xc3\xd2\x45\xce\x3c\x74\x6f\xcb\xe6\x3a\x36\x79\xd5\x1b\x6a\x51\x6e\xbe\xbe\x0e\x20",
  "\xc3\x4c\x4e\x18\x26\xe5\xd4\x03\xb7\x8e\x24\x6e\x88\xaa\x05\x1c\x36\xcc\xf0\xaa\xfe\xbf\xfe\x13\x7d\x14\x8a\x2b\xf9\x10\x45\x62",
  "\xc9\x40\xe5\xa4\x40\x41\x57\xcf\xb1\x62\x8b\x10\x8d\xb0\x51\xa8\xd4\x39\xe1\xa4\x21\x39\x4e\xc4\xeb\xcc\xb9\xec\x92\xa8\xac\x78",
  "\x47\xcf\xc5\x49\x7c\x53\xdc\x8e\x61\xc9\x1d\x17\xfd\x62\x6f\xfb\x1c\x49\xe2\xbc\xa9\x4e\xed\x05\x22\x81\xb5\x10\xb1\x11\x7a\x24",
  "\xf1\xc6\x16\x5d\x33\x36\x73\x51\xb0\xda\x8f\x6e\x45\x11\x01\x0c\x68\x17\x4a\x03\xb6\x58\x12\x12\xc7\x1c\x0e\x1d\x02\x6c\x3c\x72",
  "\x87\x26\x0f\x7a\x2f\x12\x49\x51\x18\x36\x0f\x02\xc2\x6a\x47\x0f\x45\x0d\xad\xf3\x4a\x41\x3d\x21\x04\x2b\x43\xb9\xd9\x3e\x13\x09",
  /* Non-square x^2 */
  "\x26\x94\x8d\x35\xca\x62\xe6\x43\xe2\x6a\x83\x17\x73\x32\xe6\xb6\xaf\xeb\x9d\x08\xe4\x26\x8b\x65\x0f\x1f\x5b\xbd\x8d\x81\xd3\x71",
  "\x4e\xac\x07\x7a\x71\x3c\x57\xb4\xf4\x39\x76\x29\xa4\x14\x59\x82\xc6\x61\xf4\x80\x44\xdd\x3f\x96\x42\x7d\x40\xb1\x47\xd9\x74\x2f",
  "\xde\x6a\x7b\x00\xde\xad\xc7\x88\xeb\x6b\x6c\x8d\x20\xc0\xae\x96\xc2\xf2\x01\x90\x78\xfa\x60\x4f\xee\x5b\x87\xd6\xe9\x89\xad\x7b",
  "\xbc\xab\x47\x7b\xe2\x08\x61\xe0\x1e\x4a\x0e\x29\x52\x84\x14\x6a\x51\x01\x50\xd9\x81\x77\x63\xca\xf1\xa6\xf4\xb4\x22\xd6\x70\x42",
  "\x2a\x29\x2d\xf7\xe3\x2c\xab\xab\xbd\x9d\xe0\x88\xd1\xd1\xab\xec\x9f\xc0\x44\x0f\x63\x7e\xd2\xfb\xa1\x45\x09\x4d\xc1\x4b\xea\x08",
  "\xf4\xa9\xe5\x34\xfc\x0d\x21\x6c\x44\xb2\x18\xfa\x0c\x42\xd9\x96\x35\xa0\x12\x7e\xe2\xe5\x3c\x71\x2f\x70\x60\x96\x49\xfd\xff\x22",
  "\x82\x68\x43\x6f\x8c\x41\x26\x19\x6c\xf6\x4b\x3c\x7d\xdb\xda\x90\x74\x6a\x37\x86\x25\xf9\x81\x3d\xd9\xb8\x45\x70\x77\x25\x67\x31",
  "\x28\x10\xe5\xcb\xc2\xcc\x4d\x4e\xec\xe5\x4f\x61\xc6\xf6\x97\x58\xe2\x89\xaa\x7a\xb4\x40\xb3\xcb\xea\xa2\x19\x95\xc2\xf4\x23\x2b",
  /* Negative xy value */
  "\x3e\xb8\x58\xe7\x8f\x5a\x72\x54\xd8\xc9\x73\x11\x74\xa9\x4f\x76\x75\x5f\xd3\x94\x1c\x0a\xc9\x37\x35\xc0\x7b\xa1\x45\x79\x63\x0e",
  "\xa4\x5f\xdc\x55\xc7\x64\x48\xc0\x49\xa1\xab\x33\xf1\x70\x23\xed\xfb\x2b\xe3\x58\x1e\x9c\x7a\xad\xe8\xa6\x12\x52\x15\xe0\x42\x20",
  "\xd4\x83\xfe\x81\x3c\x6b\xa6\x47\xeb\xbf\xd3\xec\x41\xad\xca\x1c\x61\x30\xc2\xbe\xee\xe9\xd9\xbf\x06\x5c\x8d\x15\x1c\x5f\x39\x6e",
  "\x8a\x2e\x1d\x30\x05\x01\x98\xc6\x5a\x54\x48\x31\x23\x96\x0c\xcc\x38\xae\xf6\x84\x8e\x1e\xc8\xf5\xf7\x80\xe8\x52\x37\x69\xba\x32",
  "\x32\x88\x84\x62\xf8\xb4\x86\xc6\x8a\xd7\xdd\x96\x10\xbe\x51\x92\xbb\xea\xf3\xb4\x43\x95\x1a\xc1\xa8\x11\x84\x19\xd9\xfa\x09\x7b",
  "\x22\x71\x42\x50\x1b\x9d\x43\x55\xcc\xba\x29\x04\x04\xbd\xe4\x15\x75\xb0\x37\x69\x3c\xef\x1f\x43\x8c\x47\xf8\xfb\xf3\x5d\x11\x65",
  "\x5c\x37\xcc\x49\x1d\xa8\x47\xcf\xeb\x92\x81\xd4\x07\xef\xc4\x1e\x15\x14\x4c\x87\x6e\x01\x70\xb4\x99\xa9\x6a\x22\xed\x31\xe0\x1e",
  "\x44\x54\x25\x11\x7c\xb8\xc9\x0e\xdc\xbc\x7c\x1c\xc0\xe7\x4f\x74\x7f\x2c\x1e\xfa\x56\x30\xa9\x67\xc6\x4f\x28\x77\x92\xa4\x8a\x4b",
  /* s = -1, which causes y = 0 */
  "\xec\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f",
};

int
main( int     argc,
      char ** argv ) {
  fd_boot( &argc, &argv );

  /* Reject bad encodings */
  for( uchar const * s  = *bad_encodings;
                     s  < (uchar const *)bad_encodings + sizeof bad_encodings;
                     s += 32 ) {
    fd_ed25519_ge_p3_t h[1];
    if( FD_UNLIKELY( !!fd_ristretto255_ge_frombytes_vartime( h, s ) ) ) {
      FD_LOG_ERR(( "FAIL"
                   "\n\tBad encoding was not rejected:"
                   "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
                   FD_LOG_HEX16_FMT_ARGS( s ), FD_LOG_HEX16_FMT_ARGS( s+16 ) ));
    }
  }

  FD_LOG_NOTICE(( "pass" ));
  fd_halt();
  return 0;
}

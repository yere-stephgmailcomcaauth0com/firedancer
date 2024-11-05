#include "fd_tar.h"
#include "../fd_util.h"
#include "../../flamenco/types/fd_types.h"

#include <complex.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static char null_tar_block[ FD_TAR_BLOCK_SZ ] = {0};

fd_tar_writer_t *
fd_tar_writer_new( void *       mem,
                   char const * tarball_name ) {

  if( FD_UNLIKELY( !mem ) ) {
    FD_LOG_WARNING(( "NULL mem" ));
  }

  if( FD_UNLIKELY( !fd_ulong_is_aligned( (ulong)mem, fd_tar_writer_align() ) ) ) {
    FD_LOG_WARNING(( "unaligned mem" ));
  }

  fd_tar_writer_t * writer = (fd_tar_writer_t *)mem;

  /* Create and open the file */
  int fd = open( tarball_name, O_CREAT | O_RDWR, 0644 );
  if( FD_UNLIKELY( fd==-1 ) ) {
    FD_LOG_WARNING(( "Failed to open and create tarball (%i-%s)", errno, fd_io_strerror( errno ) ));
  }

  int err = ftruncate( fd, 0UL );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(( "Failed to truncate tarball (%i-%s)", errno, fd_io_strerror( errno ) ));
  }

  writer->fd         = fd;
  writer->header_pos = ULONG_MAX;
  writer->data_sz    = ULONG_MAX;

  return writer;
}

void *
fd_tar_writer_delete( fd_tar_writer_t * writer ) {

  /* Write out tar trailer blocks */
  ulong out_sz = 0UL;
  int err = fd_io_write( writer->fd, null_tar_block, FD_TAR_BLOCK_SZ, FD_TAR_BLOCK_SZ, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(( "Failed to write out the first tar trailer (%i-%s)", errno, fd_io_strerror( errno ) ));
    return NULL;
  }
  err = fd_io_write( writer->fd, null_tar_block, FD_TAR_BLOCK_SZ, FD_TAR_BLOCK_SZ, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(( "Failed to write out the final tar trailer (%i-%s)", errno, fd_io_strerror( errno ) ));
    return NULL;
  }

  /* Close the file descriptor */
  err = close( writer->fd );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(( "Failed to close the tarball file descriptor (%i-%s)", errno, fd_io_strerror( errno ) ));
    return NULL;
  }

  return (void*)writer;
}

int
fd_tar_writer_new_file( fd_tar_writer_t * writer,
                        char const *      file_name ) {

  /* Save position of the header in the file and do simple sanity checks. */

  writer->header_pos = (ulong)lseek( writer->fd, 0, SEEK_CUR );
  FD_LOG_WARNING(("NEW FILE %lu %lu", writer->header_pos, writer->header_pos + 512 ));

  if( FD_UNLIKELY( !fd_ulong_is_aligned( writer->header_pos, FD_TAR_BLOCK_SZ ) ) ) {
    FD_LOG_WARNING(("Unaligned header position %lu", writer->header_pos ));
    return -1;
  }
  
  /* Populate what fields you can in the header */
  fd_tar_meta_t meta = {0};

  /* Copy in file name */
  fd_memcpy( &meta.name, file_name, strlen( file_name ) );

  /* Copy in the mode: it will always be 0644 and will be left padded.
     TODO: make this mode configurable in the future*/
  fd_memcpy( &meta.mode, "0000644\0", sizeof(meta.mode) );
  
  /* Copy in the magic and version */
  fd_memcpy( &meta.magic, "ustar  \0", sizeof(meta.magic) + sizeof(meta.version) );

  /* Write in the temporary value for the checksum*/
  fd_memcpy( &meta.chksum, "        ", sizeof(meta.chksum) );

  ulong out_sz = 0UL;
  int err = fd_io_write( writer->fd, &meta, FD_TAR_BLOCK_SZ, FD_TAR_BLOCK_SZ, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING((" Failed to write out the header (%i-%s)", errno, fd_io_strerror( errno ) ));
    return -1;
  }

  if( FD_UNLIKELY( out_sz!=FD_TAR_BLOCK_SZ ) ) {
    FD_LOG_WARNING(("Failed to write out correct size header (%lu)", out_sz ));
    return -1;
  }

  /* Now that the header is written out, upate the data pos and file sz */

  writer->data_sz  = 0UL;

  return 0;
}

int
fd_tar_writer_stream_file_data( fd_tar_writer_t * writer,
                                void const *      data,
                                ulong             data_sz ) {
  
  ulong out_sz = 0UL;
  //FD_LOG_WARNING(("CURRENT FILE POINTER BEFORE WRITE %lu", (ulong)lseek( writer->fd, 0, SEEK_CUR ) ));
  int   err    = fd_io_write( writer->fd, data, data_sz, data_sz, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(("Failed to write out the data (%i-%s)", errno, fd_io_strerror( errno ) ));
    return -1;
  }
  if( FD_UNLIKELY( out_sz!=data_sz ) ) {
    FD_LOG_WARNING(("Failed to write out the data (%lu)", out_sz ));
    return -1;
  }

  writer->data_sz += data_sz;

  //FD_LOG_WARNING(("CURRENT FILE POINTER AFTER WRITE %lu", (ulong)lseek( writer->fd, 0, SEEK_CUR ) ));

  return 0;
}

int
fd_tar_writer_fini_file( fd_tar_writer_t * writer ) {

  /* If the current file that has been written out does not meet the tar
     alignment requirements, pad out the rest of the file and update the
     header with the file sz and checksum. */


  ulong out_sz   = 0UL;
  ulong align_sz = fd_ulong_align_up( writer->data_sz, FD_TAR_BLOCK_SZ ) - writer->data_sz;
  int   err      = fd_io_write( writer->fd, null_tar_block, align_sz, align_sz, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(("Failed to write out the padding (%i-%s)", errno, fd_io_strerror( errno ) ));
    return -1;
  }

  FD_LOG_WARNING(("WRITER DATA SIZE %lu ALIGN %lu", writer->data_sz, align_sz));

  if( FD_UNLIKELY( out_sz!=align_sz ) ) {
    FD_LOG_WARNING(("Failed to write out the correct size padding (%lu)", out_sz ));
    return -1;
  }

  /* Now we need to write back to the header of the file. This involves
     first setting the file pointer to where we expect the header to be.  */
  ulong prev_pos = (ulong)lseek( writer->fd, 0, SEEK_CUR );

  ulong seek = (ulong)lseek( writer->fd, (long)writer->header_pos, SEEK_SET );
  if( FD_UNLIKELY( seek!=writer->header_pos ) ) {
    FD_LOG_WARNING(("Failed to seek to the header position (%lu)", seek ));
    return -1;
  }

  fd_tar_meta_t meta = {0};
  err = fd_io_read( writer->fd, &meta, FD_TAR_BLOCK_SZ, FD_TAR_BLOCK_SZ, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(("Failed to write out the header (%i-%s)", errno, fd_io_strerror( errno ) ));
    return -1;
  }
  if( FD_UNLIKELY( out_sz!=FD_TAR_BLOCK_SZ ) ) {
    FD_LOG_WARNING(("Failed to write out the correct size header (%lu)", out_sz ));
    return -1;
  }

  /* Now that the file is read in, update the size in the header. */

  fd_tar_meta_set_size( &meta, writer->data_sz );

  /* Write in the checksum which is left padded with zeros */

  uint checksum = 0UL;
  for( ulong i=0UL; i<FD_TAR_BLOCK_SZ; i++ ) {
    checksum += ((uchar *)&meta)[i];
  }

  meta.chksum[7] = '\0';
  snprintf( meta.chksum, 8UL, "%07o", checksum );

  /* Seek back to the start of the header block */
  lseek( writer->fd, (long)writer->header_pos, SEEK_SET );
  err = fd_io_write( writer->fd, &meta, FD_TAR_BLOCK_SZ, FD_TAR_BLOCK_SZ, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(("Failed to write out the header (%i-%s)", errno, fd_io_strerror( errno ) ));
    return -1;
  }
  if( FD_UNLIKELY( out_sz!=FD_TAR_BLOCK_SZ ) ) {
    FD_LOG_WARNING(("Failed to write out the correct size header (%lu)", out_sz ));
    return -1;
  }

  /* Reset the file pointer to the end of the file so that we can continue writing
     out the next file. */

  seek = (ulong)lseek( writer->fd, 0, SEEK_END );
  if( FD_UNLIKELY( seek!=prev_pos ) ) {
    return -1;
  }
  FD_LOG_WARNING(("END OF FILE %lu", seek));

  writer->data_sz = ULONG_MAX;
 
  return 0;
}

int
fd_tar_writer_make_space( fd_tar_writer_t * writer, ulong data_sz ) {
  long file_sz = lseek( writer->fd, 0, SEEK_END );
  FD_LOG_WARNING(("MAKING SPACE %lu", file_sz));
  int err = ftruncate( writer->fd, file_sz + (long)data_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(("Failed to make space in the tarball (%i-%s)", errno, fd_io_strerror( errno ) ));
    return -1;
  }
  long new_sz = lseek( writer->fd, 0, SEEK_END );
  if( FD_UNLIKELY( new_sz != file_sz + (long)data_sz ) ) {
    FD_LOG_WARNING(("Failed to make space in the tarball"));
    return -1;
  }
  writer->data_sz = data_sz;
  writer->wb_pos  = (ulong)file_sz;

  FD_LOG_WARNING(("MAKING SPACE"));

  return 0;
}

int
fd_tar_writer_fill_space( fd_tar_writer_t * writer, void const * data, ulong data_sz ) {

  FD_LOG_WARNING(("FILLING SPACE AT %lu", writer->wb_pos));
  long wb_pos = lseek( writer->fd, (long)writer->wb_pos, SEEK_SET );
  if( FD_UNLIKELY( (ulong)wb_pos!=writer->wb_pos ) ) {
    FD_LOG_WARNING(("Failed to seek to the write back position (%lu %lu)", wb_pos, writer->wb_pos ));
    return -1;
  }

  ulong out_sz = 0UL;
  int err = fd_io_write( writer->fd, data, data_sz, data_sz, &out_sz );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(("Failed to write out the data (%i-%s)", errno, fd_io_strerror( errno ) ));
    return -1;
  }
  if( FD_UNLIKELY( out_sz!=data_sz ) ) {
    FD_LOG_WARNING(("Failed to write out the data (%lu)", out_sz ));
    return -1;
  }


  lseek( writer->fd, (long)writer->wb_pos, SEEK_SET );
  uchar * buf = fd_scratch_alloc( 8UL, data_sz );
  fd_io_read( writer->fd, buf, data_sz, data_sz, &out_sz  );
  fd_bincode_decode_ctx_t decode = {
    .data = buf,
    .dataend = buf + data_sz,
    .valloc = fd_scratch_virtual()
  };
  fd_solana_manifest_t manifest_two = {0};
  //FD_TEST(0 == fd_solana_manifest_serializable_decode( &manifest, &decode ));
  FD_TEST(0 == fd_solana_manifest_decode( &manifest_two, &decode ));


  writer->wb_pos = ULONG_MAX;

  FD_LOG_WARNING(("FILLING SPACE"));

  lseek( writer->fd, 0, SEEK_END );

  return 0;
}

/* THIS FILE WAS GENERATED BY make frontend. DO NOT EDIT BY HAND! */
#ifndef HEADER_fd_src_app_fdctl_run_tiles_generated_http_import_dist_h
#define HEADER_fd_src_app_fdctl_run_tiles_generated_http_import_dist_h

#include "../../../../../util/fd_util.h"


struct fd_http_static_file {
    const char * name;
    const uchar * data;
    ulong const * data_len;
    const uchar * zstd_data;
    ulong const * zstd_data_len;
};

typedef struct fd_http_static_file fd_http_static_file_t;

extern fd_http_static_file_t STATIC_FILES[11];

#endif

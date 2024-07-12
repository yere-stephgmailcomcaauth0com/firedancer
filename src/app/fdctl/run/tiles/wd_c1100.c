#include "wd_c1100.h"
#include "../../../../util/log/fd_log.h"

#include <stdio.h>
#include <stdlib.h>
#define __USE_MISC
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static
uint read_pci_resource_file(const char * pcie_device, BarInfo * bars, uint bars_sz ) {
  char resource_file[ PATH_MAX ];
  FD_TEST( fd_cstr_printf_check( resource_file, PATH_MAX, NULL, "/sys/bus/pci/devices/%s/resource", pcie_device ) );

  FILE * file = fopen( resource_file, "r" );
  if( FD_UNLIKELY( !file ) ) {
    perror( "fopen" );
    return UINT_MAX;
  }

  char line[256];
  uint bar_num = 0;
  uint count = 0;

  while( fgets( line, sizeof( line ), file ) && count < bars_sz ) {
    ulong start, end, flags;
    if( FD_LIKELY( sscanf( line, "0x%lx 0x%lx 0x%lx", &start, &end, &flags ) == 3 ) ) {
      if( FD_LIKELY( start != 0 || end != 0) ) {  // Only consider valid BARs
        bars[count].start = start;
        bars[count].end = end;
        bars[count].size = end - start + 1;
        bars[count].num = bar_num;
        strncpy( bars[count].pcie_device, pcie_device, sizeof( bars[count].pcie_device ) - 1 );
        FD_TEST( fd_cstr_printf_check( bars[count].path, sizeof( bars[count].path), NULL, "/sys/bus/pci/devices/%s/resource%d", pcie_device, bars[count].num ) );
        count++;
      }
    }
    bar_num++;
  }

  fclose(file);
  return count;
}

void print_bar_infos( BarInfo * bars, uint bars_sz ) {
  for( uint i=0UL; i<bars_sz; i++ ) {
    FD_LOG_NOTICE(( "BAR%d: Start = 0x%lx, End = 0x%lx, Size = 0x%08lx, Path=%s", i, bars[i].start, bars[i].end, bars[i].size, bars[i].path ));
  }
}

static
void print_bar_maps( BarMap * bms, uint bms_sz ) {
  for( uint i=0UL; i<bms_sz; i++ ) {
    FD_LOG_NOTICE(( "BAR %d: Start = 0x%lx, End = 0x%lx, Size = 0x%08lx, MappedAddr=0x%lx, fd=%d, Path=%s",
        i, bms[i].info.start, bms[i].info.end, bms[i].info.size, (ulong)bms[i].addr, bms[i].fd, bms[i].info.path ));
  }
}

static
void mmap_bar( BarInfo const * info, BarMap * bm ) {
  char path[ PATH_MAX ];
  FD_TEST( fd_cstr_printf_check( path, PATH_MAX, NULL, "/sys/bus/pci/devices/%s/resource0", info->pcie_device ) );
  bm->fd = open( info->path, O_RDWR | O_SYNC );

  if( FD_UNLIKELY( bm->fd == -1 ) ) {
    FD_LOG_ERR(("Error opening device file %s %s", info->path, info->pcie_device ));
    return;
  }

  bm->addr = mmap( (void *)info->start, info->size, PROT_READ | PROT_WRITE, MAP_SHARED, bm->fd, 0);

  bm->info = *info;

  if( FD_UNLIKELY( bm->addr == MAP_FAILED ) ) {
    FD_LOG_ERR(("Error mapping memory: %s\n", strerror(errno)));
    return;
  }
}

int wd_pcie_peek( void * h, ulong offset, uint * value ) {
  BarMap * bm = (BarMap *)h;
  char   * base_addr   = (char *)bm->addr;
  uint   * target_addr = (uint *)(base_addr + offset);
  *value = *target_addr;
  return 0;
}

int wd_pcie_poke( void * h, ulong offset, uint   value ) {
  BarMap * bm = (BarMap *)h;
  char   * base_addr   = (char *)bm->addr;
  uint   * target_addr = (uint *)(base_addr + offset);
  *target_addr = value;
  return 0;
}

static
void * get_bar_handle( uint bar_num, BarMap const * bm, uint bm_sz ) {
  for( uint i=0; i<bm_sz; i++ ) {
    if( FD_UNLIKELY( bm[i].info.num == bar_num ) ) {
      return (void *)&bm[i];
    }
  }
  return NULL;
}

int c1100_init( C1100 * c1100, const char * pcie_device ) {
  BarInfo bar_infos[ MAX_BARS ];
  c1100->sz = read_pci_resource_file( pcie_device, bar_infos, MAX_BARS );
  for( uint i=0; i<c1100->sz; i++ ) {
    mmap_bar( &bar_infos[i], &c1100->bm[i] );
  }
  print_bar_maps( c1100->bm, c1100->sz );
  return 0;
}

void * c1100_bar_handle( C1100 const * c1100, uint bar_num ) {
  return get_bar_handle( bar_num, c1100->bm, c1100->sz );
}

uint c1100_bar_count( C1100 const * c1100 ) {
  return c1100->sz;
}

int c1100_bar_fd( C1100 * c1100, uint bar_num ) {
  for( uint i=0; i<c1100->sz; i++ ) {
    if( FD_UNLIKELY( c1100->bm[i].info.num == bar_num ) ) {
      return c1100->bm[i].fd;
    }
  }
  return -1;
}

#include <stdint.h>
ulong _wd_get_phys(void * p)
{
    ulong PAGE_SIZE = (ulong)sysconf(_SC_PAGESIZE);
    int pagemap_fd;
    uint64_t vaddr;
    uintptr_t vpn;
    uint64_t pfn;

    pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0)
    {
        FD_LOG_ERR (( "cannot open pagemap file: %d ", pagemap_fd ));
        return 0;
    }

    vaddr = (uint64_t)p;
    vpn = vaddr / PAGE_SIZE;
    for (size_t nread = 0; nread < sizeof(pfn); )
    {
        ssize_t ret = pread(pagemap_fd, &pfn, sizeof(pfn) - nread, (off_t)((vpn * sizeof(pfn)) + nread));
        if (ret <= 0)
        {
            FD_LOG_ERR (( "pread error: %lu ", ret ));
            close(pagemap_fd);
            return 0;
        }
        nread += (size_t)ret;
    }
    pfn &= (1UL << 55) - 1;
    pfn = (pfn * (long unsigned int)PAGE_SIZE) + (vaddr % (long unsigned int)PAGE_SIZE);

    close(pagemap_fd);

    return pfn;
}

uint c1100_dma_enabled( C1100 * c1100 ) {
  void * handle = c1100_bar_handle( c1100, 0 );

  uint value;
  wd_pcie_peek( handle, 0x0000, &value );
  return value;
}

void *
wd_alloc( ulong len, ulong * dma_addr ) {
  void * buf = mmap(0, len, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, -1, 0);
  *dma_addr = _wd_get_phys( buf );
  return buf;
}

void c1100_interrupts_enable( C1100 * c1100 ) {
  void * handle = c1100_bar_handle( c1100, 0 );
  wd_pcie_poke( handle, 0x0008, 3 );
}

int c1100_dma_test( C1100 * c1100, void * buf, ulong dma_addr ) {
  FD_LOG_NOTICE(( "virt: %lu phys: %lu", (ulong)buf, dma_addr ));

  for( uint i=0; i<256; i++ ) {
      ((char *)buf)[i] = (char)i;
  }
  FD_LOG_HEXDUMP_NOTICE(( "data", buf, 256 ));

  FD_LOG_NOTICE(( "dma enabled: %u", c1100_dma_enabled( c1100 ) ) );

  c1100_interrupts_enable( c1100 );

  void * bar2 = c1100_bar_handle( c1100, 2 );
  uint value;
  wd_pcie_poke( bar2, 0, 0 );
  wd_pcie_peek( bar2, 0, &value );
  FD_LOG_NOTICE(( "bar2 %08x", value ));
  wd_pcie_poke( bar2, 0, 0x11223344 );
  wd_pcie_peek( bar2, 0, &value );
  FD_LOG_NOTICE(( "bar2 %08x", value ));

  void * bar0 = c1100_bar_handle( c1100, 0 );

  FD_LOG_NOTICE(( "start copy to card" ));
  wd_pcie_poke( bar0, 0x000100, (uint)((dma_addr + 0x0000) & 0xffffffff ) );
  wd_pcie_poke( bar0, 0x000104, (uint)(((dma_addr + 0x0000) >> 32) & 0xffffffff ) );
  wd_pcie_poke( bar0, 0x000108, 0x100 );
  wd_pcie_poke( bar0, 0x00010C, 0x0 );
  wd_pcie_poke( bar0, 0x000110, 0x100 );
  wd_pcie_poke( bar0, 0x000114, 0xAA );

  usleep(1000);

  FD_LOG_NOTICE(( "read status" ));
  wd_pcie_peek( bar0, 0x000000, &value );
  FD_LOG_NOTICE(( "%08x", value ));

  wd_pcie_peek( bar0, 0x000118, &value );
  FD_LOG_NOTICE(( "%08x", value ));

  FD_LOG_NOTICE(( "start copy to host" ));
  wd_pcie_poke( bar0, 0x000200, (uint)((dma_addr + 0x0200) & 0xffffffff ) );
  wd_pcie_poke( bar0, 0x000204, (uint)(((dma_addr + 0x0200) >> 32) & 0xffffffff ) );
  wd_pcie_poke( bar0, 0x000208, 0x100 );
  wd_pcie_poke( bar0, 0x00020C, 0x0 );
  wd_pcie_poke( bar0, 0x000210, 0x100 );
  wd_pcie_poke( bar0, 0x000214, 0x55 );

  usleep( 1000 );

  FD_LOG_NOTICE(( "Read status" ));

  wd_pcie_peek( bar0, 0x000000, &value );
  FD_LOG_NOTICE(( "%08x", value ));

  wd_pcie_peek( bar0, 0x000218, &value );
  FD_LOG_NOTICE(( "%08x", value ));

  FD_LOG_NOTICE(( "read test data" ));
//   FD_LOG_HEXDUMP_NOTICE(( "data", &((char *)buf)[0x200], 256 ));

  FD_LOG_HEXDUMP_NOTICE(( "data", buf, 1024 ));

  if( memcmp( buf, &((char *)buf)[0x200], 256 ) == 0 ) {
    FD_LOG_NOTICE(( "test data matches" ));
    return 0;
  } else {
    FD_LOG_NOTICE(( "test data mismatch" ));
    return 1;
  }
}
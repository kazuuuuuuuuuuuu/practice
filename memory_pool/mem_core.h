#ifndef _NGX_CORE_H_INCLUDED_
#define _NGX_CORE_G_INCLUDED_

#define NGX_OK        0
#define NGX_ERROR    -1
#define NGX_AGAIN    -2
#define NGX_BUSY     -3
#define NGX_DONE     -4
#define NGX_DECLINED -5
#define NGX_ABORT    -6

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef intptr_t ngx_int_t;
typedef uintptr_t ngx_uint_t;

#define NGX_HAVE_POSIX_MEMALIGN 1
#define NGX_DEBUG 1
#define NGX_ALIGNMENT sizeof(unsigned long) // platform word

// align -> align to 8 -> add 7 and zero out the last three bits (0..111 -> 7) 
#define ngx_align(d, a) ( ((d) + (a-1)) & ~(a-1) )
#define ngx_align_ptr(p, a) (u_char *) (( (uintptr_t) (p) + ((uintptr_t)a - 1 )) & ~((uintptr_t)a - 1))


#include "mem_alloc.h"
#include "mem_pool_palloc.h"

#endif
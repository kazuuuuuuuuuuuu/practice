#ifndef _NGX_ALLOC_H_INCLUDED_
#define _NGX_ALLOC_H_INCLUDED_

#include "mem_core.h"

void *ngx_alloc(size_t size);
void *ngx_calloc(size_t size);

// Conditional Preprocessing Instructions
#if(NGX_HAVE_POSIX_MEMALIGN||NGX_HAVE_MEMALIGN)
void *ngx_memalign(size_t alignment, size_t size);
#else
#define ngx_memalign(alignment, size) ngx_alloc(size)
#endif

extern ngx_uint_t ngx_pagesize;
extern ngx_uint_t ngx_pagesize_shift;
extern ngx_uint_t ngx_cacheline_size;

#endif
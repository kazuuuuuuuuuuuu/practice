#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_

#include "mem_core.h"

#define NGX_MAX_ALLOC_FROM_POOL (ngx_pagesize - 1)
#define NGX_DEFAULT_POOL_SIZE (16*1024)
#define NGX_POOL_ALIGNMENT 16
#define NGX_MIN_POOL_SIZE ngx_align((sizeof(ngx_pool_t) + 2*sizeof(ngx_pool_large_t)), NGX_POOL_ALIGNMENT)

// big memory block
typedef struct ngx_pool_large_s ngx_pool_large_t;
struct ngx_pool_large_s
{
	ngx_pool_large_t *next; // point to the next big memory block
	void *alloc; // start address of the big memory block
};

typedef struct ngx_pool_s ngx_pool_t;
// data memory block structure
typedef struct
{
	u_char *last; // start address of the memory available in the memory block
	u_char *end; // end address fo the memory block
	ngx_pool_t *next; // next memory block
	ngx_uint_t failed; // the number of failed memory allocation attempts
}ngx_pool_data_t;

// first memory block structure
struct ngx_pool_s 
{
	ngx_pool_data_t d; // normal memory block struct
	size_t max; // the maximum size of memory allocatable in the current memory block
	ngx_pool_t *current; // the memory block currently used
	ngx_pool_large_t *large; // point to the big memory block
};

ngx_pool_t *ngx_create_pool(size_t size);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);

#endif
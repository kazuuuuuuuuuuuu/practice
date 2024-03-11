#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_

#include "mem_core.h"

#define NGX_MAX_ALLOC_FROM_POOL (ngx_pagesize - 1)
#define NGX_POOL_ALIGNMENT 16
#define NGX_MIN_POOL_SIZE ngx_align((sizeof(ngx_pool_t) + 2*sizeof(ngx_pool_large_t)), NGX_POOL_ALIGNMENT)

// 1 big memory block structure
typedef struct ngx_pool_large_s ngx_pool_large_t;
struct ngx_pool_large_s
{
	ngx_pool_large_t *next; // point to the next one (linked list)
	void *alloc; // start address of the allocated memory
};

typedef struct ngx_pool_s ngx_pool_t;
// 2 normal memory block structure
typedef struct
{
	u_char *last; // start address of the memory available
	u_char *end; // end address fo the memory block
	ngx_pool_t *next; // point to the next one (linked list)
	ngx_uint_t failed; // the number of memory allocation attempts failed
}ngx_pool_data_t;

// 3 head block structure
struct ngx_pool_s 
{
	ngx_pool_data_t d; // normal memory block struct
	size_t max; // the maximum size of memory available -> Decide whether to apply for a big memory block
	ngx_pool_t *current; // point to the normal memory block being used
	ngx_pool_large_t *large; // point to the head of the big memory block linked list
};

// 注意：current和next指向的都是ngx_pool_t 即head block structure 
// 所有普通块大小相同 但是第一块使用head block structure 其他内存块使用normal memory block structure

ngx_pool_t *ngx_create_pool(size_t size);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);

#endif
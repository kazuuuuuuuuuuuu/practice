#include "mem_core.h"

static inline void *ngx_palloc_small(ngx_pool_t *pool, size_t size, ngx_uint_t align);
static void *ngx_palloc_block(ngx_pool_t *pool, size_t size);
static void *ngx_palloc_large(ngx_pool_t *pool, size_t size);

ngx_pool_t *ngx_create_pool(size_t size)
{
	ngx_pool_t *p;
	p = (ngx_pool_t *)ngx_memalign(NGX_POOL_ALIGNMENT, size);
	if(p==NULL)
	{
		return NULL;
	}

	p->d.last = (u_char *) p + sizeof(ngx_pool_t);
	p->d.end = (u_char *) p + size;
	p->d.next = NULL;
	p->d.failed = 0;

	size = size - sizeof(ngx_pool_t);
	p->max = (size<NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;
	p->current = p; 
	p->large = NULL;

	return p;
}

void *ngx_palloc(ngx_pool_t *pool, size_t size)
{
	if(size<=pool->max)
	{
		return ngx_palloc_small(pool, size, 1);
	}	
	return ngx_palloc_large(pool, size);
}

static void *ngx_palloc_large(ngx_pool_t *pool, size_t size)
{
	void *p;
	ngx_uint_t n;
	ngx_pool_large_t *large;

	// 1 allocate a memory
	p = ngx_alloc(size);
	if(p==NULL)
	{
		return NULL;
	}

	// 2 mount it to the linked list
	// find a large node that has not been allocated
	n = 0;
	for(large=pool->large;large;large=large->next)
	{
		if(large->alloc==NULL)
		{
			large->alloc = p;
			return p;
		}

		// try most 3 times
		if(n>3)
		{
			n ++;
			break;
		}
	}

	// create a large node
	large = (ngx_pool_large_t *)ngx_palloc_small(pool, sizeof(ngx_pool_large_t), 1);
	if(large == NULL)
	{
		free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}

static inline void *ngx_palloc_small(ngx_pool_t *pool, size_t size, ngx_uint_t align)
{
	u_char *m;
	ngx_pool_t *p;

	p = pool->current;

	do
	{
		m = p->d.last;
		if(align)
		{
			m = ngx_align_ptr(m, NGX_ALIGNMENT);
		}
		// 1 current block is enough
		if((size_t)(p->d.end-m) >=size)
		{
			p->d.last = m + size;
			return m;
		}
		// 2 try next one
		p = p->d.next;
	}while(p);
	// 3 all blocks are NOT enough, create a new block
	return ngx_palloc_block(pool, size);
}

// create a new block
static void *ngx_palloc_block(ngx_pool_t *pool, size_t size)
{
	u_char *m;
	size_t psize;
	ngx_pool_t *p, *newp;
	psize = (size_t) (pool->d.end - (u_char *)pool);

	// 1 allocate a new memory block
	m = (u_char *)ngx_memalign(NGX_POOL_ALIGNMENT, psize);
	if(m==NULL)
	{
		return NULL;
	}
	newp = (ngx_pool_t *) m;
	newp->d.end = m + psize;
	newp->d.next = NULL;
	newp->d.failed = 0;

	// 2 add the header -> only add the data memory block structure
	m += sizeof(ngx_pool_data_t);
	m = ngx_align_ptr(m, NGX_ALIGNMENT);

	// 3 leave space for allocation this time
	newp->d.last = m + size;

	// 4 update the failed value for every memory block before
	for(p=pool->current;p->d.next;p=p->d.next)
	{
		p->d.failed ++;
		if(p->d.failed>4)
		{
			pool->current = p->d.next;
		}
	}
	// p->d.next == NULL -> the last node
	// 5 mount the new one to the linked list
	p->d.next = newp;
	return m;
}

ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p)
{
	ngx_pool_large_t *l;
	// traverse the linked list
	for(l=pool->large;l;l=l->next)
	{
		if(p==l->alloc)
		{
			fprintf(stderr, "free: %p\n", l->alloc);
			free(l->alloc);
			l->alloc = NULL;

			return NGX_OK;
		}
	}
	return NGX_DECLINED;
}

void ngx_destroy_pool(ngx_pool_t *pool)
{
	ngx_pool_t *p, *n;
	ngx_pool_large_t *l;

#if(NGX_DEBUG)
	for(l=pool->large;l;l->next)
	{
		fprintf(stderr, "free: %p\n", l->alloc);
	}

	for(p=pool, n=pool->d.next;;p=n, n=n->d.next)
	{
		fprintf(stderr, "free: %p, unused: %zu\n", p, p->d.end - p->d.last);
		if(n==NULL)
			break;
	}
#endif
	// free big memory block
	for(l=pool->large;l;l->next)
	{
		if(l->alloc)
		{
			free(l->alloc);
		}
	}

	// free normal memory block
	for(p=pool, n=pool->d.next;;p=n, n=n->d.next)
	{
		free(p);
		if(n==NULL)
			break;
	}
}

void ngx_reset_pool(ngx_pool_t *pool)
{
	ngx_pool_t *p;
	ngx_pool_large_t *l;

	for(l=pool->large;l;l->next)
	{
		if(l->alloc)
		{
			free(l->alloc);
		}
	}

	pool->d.last = (u_char *)pool + sizeof(ngx_pool_t);
	pool->d.failed = 0; 
	for(p=pool->d.next;p;p=p->d.next)
	{
		p->d.last = (u_char *)p + sizeof(ngx_pool_data_t);
		p->d.failed = 0; 
	}

	pool->current = pool;
	pool->large = NULL;
}
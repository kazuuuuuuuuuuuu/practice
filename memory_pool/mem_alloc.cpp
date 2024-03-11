#include "mem_core.h"

static int debug = 1;

ngx_uint_t ngx_pagesize;
ngx_uint_t ngx_pagesize_shift;
ngx_uint_t ngx_cacheline_size;

// using malloc to allocate memory
void *ngx_alloc(size_t size)
{
	void *p;
	p = malloc(size);
	if(p==NULL)
	{
		fprintf(stderr, "malloc failed\n");
	}
	if(debug) fprintf(stderr, "malloc: %p, %zu\n", p, size);
	return p;
}

// using ngx_alloc to allocate memory and set it to 0
void *ngx_calloc(size_t size)
{
	void *p;
	p = ngx_alloc(size);
	if(p)
	{
		memset(p, 0, size); // set to 0
	}
	return p;
}

// melloc with alignment requirment -> Select based on platform
#if(NGX_HAVE_POSIX_MEMALIGN)
// version 1
void *ngx_memalign(size_t alignment, size_t size)
{
	void *p;
	int err;
	err = posix_memalign(&p, alignment, size);
	if(err)
	{
		fprintf(stderr, "posix_memalign(%zu, %zu) failed\n", alignment, size);
		p = NULL;
	}
	if(debug) fprintf(stderr, "posix_memalign(alignment: %zu, size: %zu): %p\n", alignment, size, p);
	return p;
}
#elif(NGX_HAVE_MEMALIGN)
// version 2
void *ngx_memalign(size_t alignment, size_t size)
{
	void *p;
	p = memalign(alignment, size);
	if(p==NULL)
	{
		fprintf(stderr, "memalign(%zu, %zu) failed\n", alignment, size);
	}
	if(debug) fprintf(stderr, "memalign(alignment: %zu, size: %zu): %p\n", alignment, size, p);
	return p;
}
#endif

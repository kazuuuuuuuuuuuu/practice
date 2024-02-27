#include "mem_core.h"

#define BLOCK_SIZE 16 // the size of memory block allocated each time
#define MEM_POOL_SIZE (1024*4) // the size of memory pool block

int main(int argc, char const *argv[])
{
	int i = 0, k = 0;
	int use_free = 0;

	ngx_pagesize = getpagesize(); // global variable

	if(argc>=2)
	{
		use_free = 1;
		printf("use malloc/free\n");
	}
	else
	{
		printf("use mempool\n");
	}

	if(!use_free)
	{
		char *ptr = NULL;
		for(k=0;k<1024*500;k++)
		{
			// 1 create the memory pool
			ngx_pool_t *mem_pool = ngx_create_pool(MEM_POOL_SIZE);
			for(i=0;i<1024;i++)
			{
				// 2 allocate some memory from the pool
				ptr = (char *)ngx_palloc(mem_pool, BLOCK_SIZE);
				if(!ptr) fprintf(stderr, "ngx_palloc failed\n");
				// 3 use that memory allocated
				else
				{
					*ptr = '\0';
					*(ptr+BLOCK_SIZE-1) = '\0';
				}
			}
			// 4 free that memory pool
			ngx_destroy_pool(mem_pool);
		}
	}
	else
	{
		char *ptr[1024];
		for(k=0;k<1024*500;k++)
		{
			for(i=0;i<1024;i++)
			{
				
				ptr[i] = (char *)malloc(BLOCK_SIZE);
				if(!ptr[i]) fprintf(stderr, "malloc failed\n");
				else
				{
					*ptr[i] = '\0';
					*(ptr[i]+BLOCK_SIZE-1) = '\0';
				}
			}
			for(i=0;i<1024;i++)
			{
				if(ptr[i]) free(ptr[i]);
			}
		}
	}
	return 0;
}

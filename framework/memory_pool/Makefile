memtest : main.o mem_alloc.o mem_pool_palloc.o  
					g++ -o memtest main.o mem_alloc.o mem_pool_palloc.o 


    main.o : main.cpp mem_core.h 
		g++ -c main.cpp
    mem_alloc.o : mem_alloc.cpp mem_core.h 
		g++ -c mem_alloc.cpp
    mem_pool_palloc.o : mem_pool_palloc.cpp mem_core.h 
		g++ -c mem_pool_palloc.cpp
    clean :
		rm -fr  memtest main.o  mem_alloc.o mem_pool_palloc.o

srcs = main.cpp comm_epoll.cpp comm.cpp
cflags = -g -w

objects        := $(srcs:.cpp=.o)

epoll_demo : $(objects)                                                   
	g++ -o  epoll_demo $(objects)                                        

.cpp.o:       
	g++ $(cflags)  -c    $<

.PHONY : clean                                            
clean :                                                                  
	-rm epoll_demo  $(objects)                                         

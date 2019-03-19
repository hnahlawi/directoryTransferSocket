PORT = 50983
FLAGS = -Wall -std=gnu99 -DPORT=$(PORT)
DEPENDENCIES = hash.h ftree.h

all: rcopy_client rcopy_server

rcopy_client: rcopy_client.o ftree.o hash_functions.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<

rcopy_server: rcopy_server.o ftree.o hash_functions.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<	

clean: 
	rm *.o rcopy_client rcopy_server
 

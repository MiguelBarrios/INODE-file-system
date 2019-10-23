all:	project2 server
reset:	clean  all

project2:	project2.c 
	gcc project2.c storage_remote.c -o project2 -lm

server: server.c 
	gcc server.c  storage.c -o server -lm

pipes:
	mkfifo pipe_in pipe_out

clean:
	rm -f project2 *.o
	rm -f storage_remote *.o
	rm -f storage *.o
	rm -f server *.o

reset: clean all
	clear

zip:
	zip project2.zip project2.c server.c storage.c storage.h storage_remote.c storage_remote.h comm.h storage_common.h Makefile README.txt

rbuild: clean all
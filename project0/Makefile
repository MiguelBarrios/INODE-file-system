all:	project0 buildOne

reset:	clean  all

project0:	project0.c
	gcc project0.c -o project0

buildOne:
	./project0

buildTwo:	project0
	./project0 [u]

clean:
	rm -f project0 *.o

rbuild: clean all

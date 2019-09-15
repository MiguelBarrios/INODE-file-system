all:	project1 build

reset:	clean  all

project1:	project1.c 
	gcc project1.c -o project1

build:
	./project1

clean:
	rm -f project1 *.o
	rm -f storage *.o
	rm -f storageh *.o

zip:
	zip project1.zip *.c storage.zip *.c storage.zip *.h Makefile README.txt

rbuild: clean all

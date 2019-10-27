all:	 ouFormat inspect
reset:	clean  all



ouFormat:	oufs_format.c 
	gcc oufs_format.c oufs_lib.c oufs_lib_support.c storage.c  virtual_disk.c -o oufs -lm

inspect: oufs_inspect.c
	gcc oufs_inspect.c oufs_lib.c oufs_lib_support.c virtual_disk.c storage.c -o oufs_inspect -lm


pipes:
	mkfifo pipe_in pipe_out

clean:
	rm -f ouFormat oufs_inspect oufs *.o



zip:
	zip project3.zip oufs_format.c oufs.h oufs_inspect.c oufs_lib.c oufs_lib.h oufs_lib_support.h oufs_lib_support.c oufs_ls.c oufs_mkdir.c oufs_rmdir.c oufs_stats.c storage.c storage.h virtual_disk.h virtual_disk.c Makefile README.txt

rbuild: clean all
all:	 ouFormat inspect oufs_ls oufs_mkdir oufs_rmdir oufs_stats
reset:	clean  all

ouFormat:	oufs_format.c 
	gcc oufs_format.c oufs_lib.c oufs_lib_support.c storage.c  virtual_disk.c -o oufs_format -lm

inspect: oufs_inspect.c
	gcc oufs_inspect.c oufs_lib.c oufs_lib_support.c virtual_disk.c storage.c -o oufs_inspect -lm

oufs_ls: oufs_ls.c
	gcc oufs_ls.c virtual_disk.c oufs_lib.c storage.c oufs_lib_support.c -o oufs_ls -lm

oufs_mkdir: oufs_mkdir.c
	gcc oufs_mkdir.c virtual_disk.c oufs_lib.c storage.c oufs_lib_support.c -o oufs_mkdir -lm

oufs_rmdir: oufs_rmdir.c
	gcc oufs_rmdir.c virtual_disk.c oufs_lib.c storage.c oufs_lib_support.c -o oufs_rmdir -lm

oufs_stats: oufs_stats.c
	gcc oufs_stats.c virtual_disk.c oufs_lib.c storage.c oufs_lib_support.c -o oufs_stats -lm

pipes:
	mkfifo pipe_in pipe_out

clean:
	rm -f oufs_format oufs_inspect oufs_ls  oufs_mkdir  oufs_rmdir  oufs_stats *.o

test1:	clean all
	./oufs_format
	./oufs_mkdir foo
	./oufs_mkdir foo/bar
	./oufs_mkdir foo/baz
	./oufs_mkdir foo/bob
	./oufs_ls
	./oufs_ls foo
	./oufs_rmdir foo/baz
	./oufs_rmdir foo/bar
	./oufs_rmdir foo/bob

zip:
	zip project3.zip oufs_format.c oufs.h oufs_inspect.c oufs_lib.c oufs_lib.h oufs_lib_support.h oufs_lib_support.c oufs_ls.c oufs_mkdir.c oufs_rmdir.c oufs_stats.c storage.c storage.h virtual_disk.h virtual_disk.c Makefile README.txt

rbuild: clean all 
all:	 ouFormat inspect oufs_ls oufs_mkdir oufs_rmdir oufs_stats oufs_touch oufs_create oufs_append oufs_cat oufs_copy oufs_remove oufs_link

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

oufs_touch: oufs_touch.c
	gcc oufs_touch.c virtual_disk.c oufs_lib.c storage.c oufs_lib_support.c -o oufs_touch -lm

oufs_create: oufs_create.c
	gcc oufs_create.c oufs_lib.c oufs_lib_support.c virtual_disk.c storage.c -o oufs_create -lm

oufs_append: oufs_append.c
	gcc oufs_append.c oufs_lib.c oufs_lib_support.c virtual_disk.c storage.c -o oufs_append -lm

oufs_cat: oufs_cat.c
	gcc oufs_cat.c oufs_lib.c oufs_lib_support.c virtual_disk.c storage.c -o oufs_cat -lm

oufs_copy: oufs_copy.c
	gcc oufs_copy.c oufs_lib.c oufs_lib_support.c virtual_disk.c storage.c -o oufs_copy -lm

oufs_remove: oufs_remove.c
	gcc oufs_remove.c oufs_lib.c oufs_lib_support.c  virtual_disk.c storage.c -o oufs_remove -lm

oufs_link: oufs_link.c
	gcc oufs_link.c oufs_lib_support.c virtual_disk.c oufs_lib.c storage.c -o oufs_link -lm

pipes:
	mkfifo pipe_in pipe_out

clean:
	rm -f oufs_format oufs_inspect oufs_ls oufs_mkdir  oufs_rmdir  oufs_stats oufs_touch oufs_create oufs_append oufs_cat oufs_copy oufs_remove oufs_link *.o

zip:
	zip project4.zip oufs_format.c oufs_cat.c oufs_remove.c oufs_copy.c oufs_create.c oufs_append.c oufs_touch.c oufs.h oufs_inspect.c oufs_lib.c oufs_lib.h oufs_lib_support.h oufs_lib_support.c oufs_ls.c oufs_mkdir.c oufs_rmdir.c oufs_stats.c storage.c storage.h virtual_disk.h virtual_disk.c oufs_link.c oufs_inspect.c Makefile README.txt

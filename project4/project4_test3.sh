#!/bin/bash
# Usage: 
#   ./project4_test3.sh > aa
#   diff -y aa project4_test3.out | less
# 
# NOTE: this test assumes that you are using the stock oufs.h
#   file in the project 4 skeleton

./oufs_format
./oufs_inspect -master
cat oufs.h | ./oufs_create baz
./oufs_ls
./oufs_inspect -dblock 5
./oufs_inspect -inode 1
./oufs_cat baz > copy_of_oufs.h
./oufs_inspect -master
diff copy_of_oufs.h oufs.h

#!/bin/bash
# Usage: 
#   ./project4_test.sh > aa
#   diff -y aa project4_test.out | less

./oufs_format
./oufs_inspect -master
./oufs_touch foo
./oufs_ls
./oufs_inspect -dblock 5
./oufs_inspect -inode 1
./oufs_inspect -master

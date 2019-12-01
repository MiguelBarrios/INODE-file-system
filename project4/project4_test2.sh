#!/bin/bash
# Usage: 
#   ./project4_test2.sh > aa
#   diff -y aa project4_test2.out | less

./oufs_format
./oufs_inspect -master
echo "abc" | ./oufs_create bar
./oufs_ls
./oufs_inspect -dblock 5
./oufs_inspect -inode 1
./oufs_cat bar
./oufs_inspect -master


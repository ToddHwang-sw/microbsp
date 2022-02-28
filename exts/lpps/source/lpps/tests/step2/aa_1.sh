#!/bin/sh


mkdir -p A/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
mkdir -p B/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
mkdir -p C/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
mkdir -p D/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
echo "CREATE !!!"
sync
sync
sleep 2

\rm -rf A B C D 
echo "DELETE !!!!"
sync
sync
sleep 2


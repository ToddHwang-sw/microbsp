#!/bin/sh


mkdir -p A/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
mkdir -p B/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
mkdir -p C/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
mkdir -p D/1/2/3/4/5/6/7/8/9/0/1/2/3/4/5/6/7/8/9/0
echo "CREATE DIR !!!"
sleep 3

touch A/1/2/3/4/5/6/7/8/9/0/hello
touch A/1/2/3/4/5/6/7/8/9/hello
touch A/1/2/3/4/5/6/7/8/hello
touch A/1/2/3/4/5/6/7/hello
touch A/1/2/3/4/5/6/hello
touch A/1/2/3/4/5/hello
touch A/1/2/3/4/hello
touch A/1/2/3/hello
touch A/1/2/hello
touch A/1/hello
touch B/1/2/3/4/5/6/7/8/9/0/hello
touch B/1/2/3/4/5/6/7/8/9/hello
touch B/1/2/3/4/5/6/7/8/hello
touch B/1/2/3/4/5/6/7/hello
touch B/1/2/3/4/5/6/hello
touch B/1/2/3/4/5/hello
touch B/1/2/3/4/hello
touch B/1/2/3/hello
touch B/1/2/hello
touch B/1/hello
touch C/1/2/3/4/5/6/7/8/9/0/hello
touch C/1/2/3/4/5/6/7/8/9/hello
touch C/1/2/3/4/5/6/7/8/hello
touch C/1/2/3/4/5/6/7/hello
touch C/1/2/3/4/5/6/hello
touch C/1/2/3/4/5/hello
touch C/1/2/3/4/hello
touch C/1/2/3/hello
touch C/1/2/hello
touch C/1/hello
touch D/1/2/3/4/5/6/7/8/9/0/hello
touch D/1/2/3/4/5/6/7/8/9/hello
touch D/1/2/3/4/5/6/7/8/hello
touch D/1/2/3/4/5/6/7/hello
touch D/1/2/3/4/5/6/hello
touch D/1/2/3/4/5/hello
touch D/1/2/3/4/hello
touch D/1/2/3/hello
touch D/1/2/hello
touch D/1/hello
echo "CREATE FIL !!!"
sync
sync
sleep 3

\rm -rf A B C D 
echo "DELETE !!!!"
sync
sync
sleep 3

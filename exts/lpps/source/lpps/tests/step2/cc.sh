#!/bin/sh


mkdir -p A/1/2/3/4/5
echo "CREATE DIR !!!"
sleep 2
touch A/1/2/3/4/5/hello
touch A/1/2/3/4/hello
touch A/1/2/3/hello
touch A/1/2/hello
touch A/1/hello
echo "CREATE FIL !!!"
sync
sync
sleep 2

\rm -rf A 
echo "DELETE !!!"
sync
sync
sleep 2

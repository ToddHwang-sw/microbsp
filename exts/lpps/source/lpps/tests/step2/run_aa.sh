#!/bin/sh

cd /pps
[ -f /pps/.memory ] || touch /pps/.memory

while true
do
	##~infot/aa_1.sh
	##~infot/cc.sh
	~infot/aa.sh
	cat /pps/.memory?user
	## echo "PRINT <<<<>>>>>"
	## cat /pps/.memory
	echo "PRINT  ...................."
	sleep 2
done


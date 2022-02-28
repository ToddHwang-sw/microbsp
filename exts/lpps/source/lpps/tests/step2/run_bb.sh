#!/bin/sh

cd /pps

while true
do
	~infot/aa.sh
	[ -f /pps/.memory ] || touch /pps/.memory
	cat /pps/.memory?user
	##echo "PRINT <<<<>>>>>"
	## cat /pps/.memory
	echo "PRINT  ...................."
	sleep 2
done


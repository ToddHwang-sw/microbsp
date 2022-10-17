#!/bin/sh

APP=$1

shift 1

cmd="$APP $@"

echo "Daemon Running..."
$cmd
sleep 1
npid=`pidof $APP`

while true
do
    app_pid=`pidof $APP`
    if [ "$app_pid" = "" ]; then
		if [ -f /var/tmp/exit_pid_$npid ]; then
			echo ""
			echo "Done!!"
			echo ""
			\rm -rf /var/tmp/exit_pid_$npid
			exit 1
		fi
		echo ""
        echo "Rerunning $cmd"
		echo ""
		\rm -rf /var/tmp/$npid
        $cmd
    fi
	npid=$app_pid
    sleep 1
done


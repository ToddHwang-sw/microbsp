
##
## qemu file for X86_64
##
QEMU=qemu-system-x86_64

## default BDDIR file 
BDDIR=.

## image files 
ISONAME=$BDDIR/output.iso
SCRIPTF=$BDDIR/qemu-if.script
EXT4HDD=$BDDIR/image.ext4
EXT4CFG=$BDDIR/config.ext4
PIDF=$BDDIR/vm.pid

run_qemu() {
	[ ! -f $PIDF ] || rm $PIDF
	touch $PIDF

    $QEMU -m 4096 -nographic -smp 4 \
		-pidfile $PIDF \
        -device e1000,netdev=net0,mac=$1 \
        -netdev tap,id=net0,script=$SCRIPTF \
        -cdrom $ISONAME \
        -drive file=$EXT4HDD \
        -drive file=$EXT4CFG
}

##
## random MAC address generation 
## 
VMMAC=`tr -dc A-F0-9 < /dev/urandom | head -c 10 | sed -r 's/(..)/\1:/g;s/:$//;s/^/02:/'`

[ "$#" != "0" ] || exit
if [ "$2" != ""  ]; then 
    BDDIR=$2
fi

case "$1" in
    "run")
        run_qemu $VMMAC
        ;;
    "stop")
        sudo kill -9 `cat $PIDF`
		rm $PIDF
        ;;
    "publish")
        if [ -f $SCRIPTF -a \
                -f $EXT4HDD -a \
                -f $EXT4CFG -a \
                -f $ISONAME ]; then 
            [ -d $2 ] || mkdir -p $2
            cp -rf $SCRIPTF $2
            cp -rf $EXT4HDD $2
            cp -rf $EXT4CFG $2
            cp -rf $ISONAME $2
            cp -rf $0       $2
        fi
esac



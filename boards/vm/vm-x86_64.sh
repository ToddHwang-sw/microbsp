QEMU=qemu-system-x86_64

isoname=output.iso

BDDIR=.
EXT4HDD=$BDDIR/image.ext4
EXT4CFG=$BDDIR/config.ext4

setup_network() {
    # tunctl -u `whoami` -t $1 
    ip tuntap add $1 mode tap user `whoami`
    ip link set $1 up
    sleep 0.5s
    # brctl addif $switch $1 (use ip link instead!)
    ip link set $1 master br0
}

run_qemu() {
    $QEMU -m 4096 -nographic -smp 4 \
        -device e1000,netdev=net0,mac=$1 \
        -netdev tap,id=net0 \
        -cdrom $isoname \
        -drive file=$EXT4HDD \
        -drive file=$EXT4CFG
}

VMMAC=`tr -dc A-F0-9 < /dev/urandom | head -c 10 | sed -r 's/(..)/\1:/g;s/:$//;s/^/02:/'`

if [ "$2" != "" ]; then
    NETID=`ifconfig -a | grep tap | wc -l`
fi

case "$1" in
    "run")
        setup_network $NETID
        run_qemu $VMMAC
        ;;
    "stop")
        sudo killall $QEMU
        ;;
esac



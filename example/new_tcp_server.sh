#!/bin/sh
if [ $# -lt 1 ]; then
    echo "Usage: $0 LISTEN_PORT"
    exit 1
fi
SERVER=`cat /net/tcp/clone`
echo "announce *!$1" > /net/tcp/$SERVER/ctl
while :
do
    echo "[*] Waiting a new connection on /net/tcp/$SERVER/ ..."
    CLIENT=`cat /net/tcp/$SERVER/listen`
    [ $? != 0 ] && exit 1
    echo "[*] Accepted a connection on /net/tcp/$CLIENT/ from: `cat /net/tcp/$CLIENT/remote`"
    cat /net/tcp/$CLIENT/data &
    cat > /net/tcp/$CLIENT/data
    kill `jobs -p` > /dev/null 2>&1
done

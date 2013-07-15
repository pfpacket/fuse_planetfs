#!/bin/sh
TCP_CLONE="/net/tcp/clone"
CLONE_NUM=`cat $TCP_CLONE`
echo "connect 74.125.235.248!80" > $TCP_CLONE
DATA_FILE="/net/tcp/$CLONE_NUM/data"
printf \
"GET /index.html HTTP/1.1\r\n\
Host: www.google.co.jp\r\n\
Connection: close\r\n\r\n" \
    > $DATA_FILE
cat $DATA_FILE

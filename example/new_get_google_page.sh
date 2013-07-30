#!/bin/sh
TCP_CLONE="/net/tcp/clone"
CLONE_NUM=`cat $TCP_CLONE`
echo "connect 74.125.235.248!80" > $TCP_CLONE
DATA_FILE="/net/tcp/$CLONE_NUM/data"
echo "GET /index.html HTTP/1.1" > $DATA_FILE
echo "Host: www.google.co.jp" > $DATA_FILE
echo "Connection: close" > $DATA_FILE
echo "" > $DATA_FILE
cat $DATA_FILE

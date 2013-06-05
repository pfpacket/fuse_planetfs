#!/bin/sh
GOOGLE_NET_FILE="/net/tcp/74.125.235.248!80"
printf \
"GET /index.html HTTP/1.1\r\n\
Host: www.google.co.jp\r\n\
Connection: close\r\n\r\n" \
    > $GOOGLE_NET_FILE
cat $GOOGLE_NET_FILE
rm $GOOGLE_NET_FILE

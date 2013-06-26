#!/bin/sh
REMOTE_HOST="/net/tcp/127.0.0.1!7"
trap "rm $REMOTE_HOST" INT
echo "Hello, planetfs" > $REMOTE_HOST
cat $REMOTE_HOST
[ -f $REMOTE_HOST ] && rm $REMOTE_HOST

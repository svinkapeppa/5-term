#! /usr/bin/env bash

BLOCK=$1
LOCATION="$( hdfs fsck -blockId $BLOCK 2>/dev/null | python script.py )"
LOCAL="$( sudo -u hdfsuser ssh hdfsuser@${LOCATION} find / -name "*$BLOCK" 2>/dev/null )"
echo "${LOCATION}:${LOCAL}"

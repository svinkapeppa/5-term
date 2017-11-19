#! /usr/bin/env bash

LOCATION="$( curl --silent -i "http://mipt-master.atp-fivt.org:50700/webhdfs/v1$1?op=OPEN" | python script.py )&length=10"
curl --silent "${LOCATION}" 2>/dev/null

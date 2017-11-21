#! /usr/bin/env bash

curl -i "http://mipt-master.atp-fivt.org:50070/webhdfs/v1$1?op=OPEN" 2>/dev/null | python script.py

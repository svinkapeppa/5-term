#! /usr/bin/env bash

LOC="$( curl --silent -i http://mipt-master.atp-fivt.org:50070/webhdfs/v1$1?op=OPEN | python script.py )"
LOC="${LOC}&length=10"
curl --silent "${LOC}"

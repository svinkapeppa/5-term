#! /usr/bin/env python

import re
import sys

nodes = []

for line in sys.stdin:
    m = re.match('Block replica on datanode.*: (.*)/', line, flags=re.UNICODE)
    if m is not None:
        nodes += [ m.group(1)]
print(sorted(nodes)[0])

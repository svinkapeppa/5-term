#!/usr/bin/env python

import re
import sys

reload(sys)
sys.setdefaultencoding('utf-8')

for line in sys.stdin:
    words = line.split()
    for word in words:
	m = re.search('http://mipt-node\d+\.atp-fivt\.org', word)
	if m is not None:
	    print m.group(0)

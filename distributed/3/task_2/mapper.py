#! /usr/bin/env python

import sys
import time
import re

for line in sys.stdin:
    uid, timestamp, url, junk = line.split('\t')
    time_struct = time.gmtime(int(timestamp))
    day = time_struct.tm_yday
    year = time_struct.tm_year
    m = re.match('.*http:\/\/(?:www\.)?(.*?)\/(.*)', url, flags=re.UNICODE)
    if m is not None:
        domain = m.group(1)
        article = m.group(2)
    print '%s\t%s\t%s\t%s' % (domain, article, str(year), str(day))

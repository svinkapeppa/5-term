#! /usr/bin/env python3

import sys
import re

try:
    for line in sys.stdin:
        ip, datetime, request, size, code, info = line.replace('\n', ' ').split()
        m = re.match('Safari/(.*)', info, flags=re.UNICODE)
        if m is not None:
            info = 'Chrome/' + m.group(1)
        print('\t'.join([ip, datetime, request, size, code, info]))
except:
    print(sys.exc_info())


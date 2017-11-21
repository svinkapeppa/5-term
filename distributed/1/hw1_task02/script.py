#! /usr/bin/env python

import re
import sys

for line in sys.stdin:
    m = re.search("Location: (.*)", line, flags=re.UNICODE)
    if m:
        print m.group(1).strip() 

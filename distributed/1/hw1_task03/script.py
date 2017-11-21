#!/usr/bin/env python

import re
import sys

for line in sys.stdin:
    m = re.search('Total blocks \(validated\):\s+(\d+)', line, flags=re.UNICODE)
    if m is not None:
        print m.group(1)
        sys.exit()

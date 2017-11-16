#!/usr/bin/env python

import re
import sys

reload(sys)
sys.setdefaultencoding('utf-8')

for line in sys.stdin:
    m = re.search('Total blocks \(validated\):"\s+\d.+', line)
    if m is not None:
        n = re.search('\d+', line)
        if n is not None:
            print n.group(0)
            sys.exit()

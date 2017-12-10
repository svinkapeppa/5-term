#! /usr/bin/env python3

import sys
import re

try:
    for line in sys.stdin:
        try:
            age = int(line)
            print(100 - age)
        except:
            print(0)
except:
    print(sys.exc_info())


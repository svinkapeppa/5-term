#! /usr/bin/env python

import sys

for line in sys.stdin:
    try:
        id, text = line.split('\t', 1)
        text = text.strip(' \t')
        print '%s\t%s\t%s' % (text[0], id, str(len(text)))
    except:
        continue

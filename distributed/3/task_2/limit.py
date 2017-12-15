#! /usr/bin/env python

import sys

top = []

for line in sys.stdin:
    domain, lifespan = line.split('\t')
    top.append((float(lifespan), domain))
    top.sort(reverse=True)
    if len(top) > 10:
        top.pop()

for lifespan, domain in top:
    print '%s\t%s' % (domain, str(lifespan))

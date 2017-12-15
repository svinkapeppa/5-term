#! /usr/bin/env python

import sys

current_letter = None
top = []

for line in sys.stdin:
    letter, id, length = line.split('\t')
    if current_letter != letter:
        if current_letter is not None:
            for i in range(len(top)):
                print '%s\t%s\t%s' % (current_letter, top[i][1], str(top[i][0]))
        top = []
        current_letter = letter
    top.append((int(length), id))
    top.sort(reverse=True)
    if len(top) > 3:
        top.pop()

if current_letter is not None:
    for i in range(len(top)):
        print '%s\t%s\t%s' % (current_letter, top[i][1], str(top[i][0]))

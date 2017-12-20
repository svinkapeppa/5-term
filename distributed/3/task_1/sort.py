#! /usr/bin/env python

import sys

data = []
temp = []
current_letter = None

for line in sys.stdin:
    letter, id, length = line.split('\t')
    data.append((letter, length.strip(), id))

for i in range(len(data)):
    letter = data[i][0]
    if current_letter != letter:
        if current_letter is not None:
            temp.sort(reverse=True)
            for j in range(len(temp)):
                print '%s\t%s\t%s' % (current_letter, temp[j][1], str(temp[j][0]))
            temp = []
        current_letter = letter
    temp.append((int(data[i][1]), data[i][2]))

if current_letter != None:
    temp.sort(reverse=True)
    for j in range(len(temp)):
        print '%s\t%s\t%s' % (current_letter, temp[j][1], str(temp[j][0]))

#! /usr/bin/env python

import sys

current_domain = None
current_article = None
current_year = None
current_day = None
visit = 0
count = 0

for line in sys.stdin:
    domain, article, year, day = line.strip().split('\t')
    if current_domain != domain:
        if current_domain is not None:
            print '%s\t%s' % (current_domain, str(float(visit) / float(count)))
        current_domain = domain
        current_article = article
        current_year = year
        current_day = day
        count = 1
        visit = 1
    if current_article != article:
        current_article = article
        current_year = year
        current_day = day
        count += 1
        visit += 1
    if current_year != year:
        current_year = year
        current_day = day
        visit += 1
    if current_day != day:
        current_day = day
        visit += 1

if current_domain is not None:
    print '%s\t%s' % (domain, str(float(visit) / float(count)))

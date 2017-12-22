#! /usr/bin/env python3

import sys

from math import sqrt

def simple(n):
    return all(n % i for i in range(2, int(sqrt(n)) + 1))

def wrapper(n):
    if simple(n):
        print(n, 'is prime')
    else:
        print(n, 'is composite')
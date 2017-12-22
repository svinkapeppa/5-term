#! /usr/bin/env python3

import sys

from random import randint
from math import gcd

def mult(a, b, m):
    if b == 1:
        return a
    elif b % 2 == 0:
        temp = mult(a, int(b / 2), m)
        return (2 * temp) % m
    
    return (mult(a, b - 1, m) + a) % m

def pows(a, b, m):
    if b == 0:
        return 1
    elif b % 2 == 0:
        temp = pows(a, int(b / 2), m)
        return mult(temp, temp, m) % m 
    
    return (mult(pows(a, b - 1, m), a, m)) % m

def ferma(n):
    if n == 2:
        return True
    else:
        for i in range(100):
            temp = (randint(0, 2**15) % (n - 2)) + 2
            if gcd(temp, n) > 1:
                return False
            if  pows(temp, n - 1, n) != 1:
                return False
    
    return True

def wrapper(n):
    if ferma(n):
        print(n, 'is prime')
    else:
        print(n, 'is composite')
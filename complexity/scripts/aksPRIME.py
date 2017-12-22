#! /usr/bin/env python3

import sys
import numpy as np

from math import gcd, sqrt
from scipy.special import comb

def checkpower(i, n):
    left = 2
    right = n
    flag = False

    while left <= right and not flag:
        mid = int((left + right) / 2)
        temp = mid ** i
        if temp == n:
            flag = True
        else:
            if n < temp:
                right = int((left + right) / 2) - 1
            else:
                left = int((left + right) / 2) + 1

    return flag

def checkperfectpower(n):
    for i in np.arange(2, np.log2(n) + 1):
        if checkpower(i, n):
            return True
    
    return False

def getord(r, n, threshold):
    if gcd(r, n) > 1:
        return False
    for i in np.arange(1, threshold):
        if (n ** int(i)) % r == 1:
            return False
    
    return True

def smallestr(n):    
    rmax = max(3, int(np.ceil(np.log2(n) ** 5)))
    threshold = np.log2(n) ** 2
    for r in np.arange(2, rmax + 1):
        if getord(r, n, threshold):
            return r

def elimination(r, n):
    for a in np.arange(2, r + 1):
        if 1 < gcd(n, a) < n:
            return True

    return False

def euler(n):
    amount = 0
    for k in range(1, n + 1):
        if gcd(n, k) == 1:
            amount += 1
    
    return amount

def polynomial_coef(n, a):
    ex = []
    for i in range(n + 1):
        ex.append(comb(n, n - i, exact=True) * (a ** (n - i)))
    
    return ex[::-1]

def reduce(polynomial, r):
    for i in range(len(polynomial) - r):
        k = polynomial[i]
        polynomial[i] = 0
        polynomial[i + r] -= k
    
    return polynomial

def modul(n, r):
    for a in np.arange(1, np.floor(sqrt(euler(n)) * np.log2(n)) + 1):
        coef = polynomial_coef(n, int(a))
        coef[0] -= 1
        coef[-1] -= int(a)
        divided = reduce(coef, int(r))
        if not all(x%n == 0 for x in divided):
            return False
    
    return True

def AKS(n):
    if checkperfectpower(n):
        return False
    else:
        r = smallestr(n)
        if elimination(r, n):
            return False
        else:
            if n <= r:
                return True
            else:
                if modul(n, r):
                    return True
                else:
                    return False

def wrapper(n):
    if AKS(n):
        print(n, 'is prime')
    else:
        print(n, 'is composite')

for n in sys.stdin:
    wrapper(int(n))
#! /usr/bin/python3

import random as rd

K = int(input())
N = int(input())
x = int(input())
M = int(x*N/10)

print ("p cnf {0} {1}".format(N, M))
for i in range(M):
    l = []
    avail = [i+1 for i in range(N)]
    rd.shuffle(avail)
    for j in range(K):
        s = rd.randint(0,1)
        c = avail[j]
        if (s == 0):
            l+= [c]
        else :
            l+= [-c]
    l += [0]
    print(' '.join(map(str,l)))

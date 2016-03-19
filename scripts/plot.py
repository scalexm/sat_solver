#! /usr/bin/python

import matplotlib.pyplot as plt
import numpy as np

K = int(input())
V = int(input())
flag = str(raw_input())


med = []
Q1 = []
Q3 = []
x_axe = [0.1 * i for i in range(20,61)]
for i in range(20,61):
    l = []
    for j in range(100):
        x = float(input())
        l += [x]
    med += [np.percentile(np.array(l), 50)]
    Q3 += [np.percentile(np.array(l), 75)]
    Q1 += [np.percentile(np.array(l), 25)]

plt.title(str(K)+"-SAT "+str(V)+" variables "+flag)
plt.plot(x_axe, med, label="Mediane")
plt.plot(x_axe, Q1, label="1er quartile")
plt.plot(x_axe, Q3, label="3eme quartile")
plt.ylabel("temps d'execution (s)")
plt.xlabel("clauses par variables")
plt.yscale('log')
plt.grid(True)
plt.legend()
plt.savefig(flag+".png")

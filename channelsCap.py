import pandas as pd
import random
import csv
import os
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

stepMs = 1000
for j in ['InHM_c_new', 'InHM_b_new', 'InHM_a_new', 'Building_c_new', 'Building_b_new', 'Building_a_new']:
    fig = plt.figure(figsize=(8,3), facecolor='w')
    ax = plt.gca()
    f1 = open ('traces/channels/{}'.format(j),"r")

    BW = []
    nextTime = stepMs
    cnt = 0
    for line in f1:
        a=line.split(' ', 1)
        currTime = a[0]
        currPkts = a[1]
        if  int(currTime) > nextTime:
            # print(a[0])
            BW.append(cnt*1450*8/stepMs/1e3)
            cnt = 0
            nextTime+=stepMs
        else:
            cnt+=int(currPkts)
    f1.close()
    print('Channel {} - peak BW {}'.format(j, max(BW)))
    xs = range(len(BW))
    xs = [i * stepMs/1000 for i in xs]
    ax.plot(xs, BW,color='r')
    # plt.xlim(0, 60)
    plt.ylim(0, None)
    plt.ylabel("Throughput (Mbps)")
    plt.xlabel("Time (s)")
    plt.savefig('traces/channels/{}.pdf'.format(j))

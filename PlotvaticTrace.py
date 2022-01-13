import os
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import sys
from glob import glob
import pandas as pd
import seaborn as sns
import math

sns.set_style("white")

def GetBaseTime (fn):
  with open(fn) as f:
    for line in f:
      line = line.strip()
      if line.startswith('# init timestamp'):
        return int(line.split(":")[-1])
  return -1


def PlotTraffic(dataPath, filePrefix, baseTimeStamp, plotStepMs):
  fileNameThput = '{}/{}_recv.dat'.format(dataPath, filePrefix)
  fileNameRtt = '{}/{}_rtts.dat'.format(dataPath, filePrefix)
  if not os.path.isfile(fileNameThput): 
    print('File {} not found!'.format(fileNameThput))
    return
  if not os.path.isfile(fileNameRtt): 
    print('File {} not found!'.format(fileNameRtt))
    return  
  fig, ax = plt.subplots(2, 1, figsize=(15,10), facecolor='w')  
  PlotThput(fileNameThput, ax[0], baseTimeStamp, plotStepMs)
  PlotRtts(fileNameRtt, ax[1], baseTimeStamp, plotStepMs)
  plt.savefig('{}/{}_traffic.pdf'.format(dataPath, filePrefix))

def PlotThput(filename, ax, baseTimeStamp, plotStepMs):
  BW = []
  times = []
  fh = open (filename,"r")
  cnt = 0
  nextTime = -1 
  for line in fh:
    sp = line.split('\t')
    pktIdx, time, pktSize = [float(x) for x in sp]
    if nextTime < 0:
      nextTime = time+plotStepMs
    if time > nextTime:
      nBits = cnt*8
      BW.append(nBits/(plotStepMs*1e3))
      cnt = pktSize
      nextTime+=plotStepMs
      times.append(time - baseTimeStamp)
    else:
      cnt+=pktSize
  nBits = cnt*8
  BW.append(nBits/(plotStepMs*1e3))    
  times.append(time- baseTimeStamp)
  fh.close()
  maxY = max(BW)
  xs = [i/1000 for i in times]
  ax.plot(xs, BW,color='r')
  ax.set_ylim(0, maxY*1.1)
  ax.set_ylabel("Throughput (Mbps)")
  ax.set_xlabel("Time (s)")
  

def PlotRtts(filename, ax, baseTimeStamp, plotStepMs):
  RTT = []
  times = []
  fh = open (filename,"r")
  for line in fh:
    sp = line.split('\t')
    time, rtt, idx = [float(x) for x in sp]
    RTT.append(rtt)
    times.append(time- baseTimeStamp)
  fh.close()
  maxY = max(RTT)
  xs = [i/1000 for i in times]
  ax.plot(xs, RTT, color='r')
  ax.set_ylim(0, maxY*1.1)
  ax.set_ylabel("RTT (ms)")
  ax.set_xlabel("Time (s)")

def PlotVatic(dataPath, filePrefix, baseTimeStamp):
  fileNameCwnd = '{}/{}_cwnd.dat'.format(dataPath, filePrefix)
  fileNameAlpha = '{}/{}_alpha.dat'.format(dataPath, filePrefix)
  fileNameCentroids = '{}/{}_centroids.dat'.format(dataPath, filePrefix)

  if not os.path.isfile(fileNameCwnd): 
    print('File {} not found!'.format(fileNameCwnd))
    return
  if not os.path.isfile(fileNameAlpha): 
    print('File {} not found!'.format(fileNameAlpha))
    return
  if not os.path.isfile(fileNameCwnd): 
    print('File {} not found!'.format(fileNameCwnd))
    return

  fig, ax = plt.subplots(3, 1, figsize=(15,10), facecolor='w')

  CWND = []
  USED = []
  FREE = []
  timesCwnd = []
  fh = open (fileNameCwnd,"r")
  for line in fh:
    sp = line.split('\t')
    time, cwnd, used, free = [float(x) for x in sp]
    CWND.append(cwnd)
    USED.append(used)
    FREE.append(free)
    timesCwnd.append(time - baseTimeStamp)
  fh.close()

  ALPHA = []
  timesAlpha = []
  fh = open (fileNameAlpha,"r")
  for line in fh:
    sp = line.split('\t')
    time, alpha = [float(x) for x in sp]
    ALPHA.append(alpha)
    timesAlpha.append(time- baseTimeStamp)
  fh.close()

  CENTROID_LOW = []
  CENTROID_HIGH = []
  timesCentroids = []
  fh = open (fileNameCentroids,"r")
  for line in fh:
    sp = line.split('\t')
    time, clow, chigh = [float(x) for x in sp]
    CENTROID_LOW.append(clow)
    CENTROID_HIGH.append(chigh)
    timesCentroids.append(time- baseTimeStamp)
  fh.close()

  maxY = max(CWND)
  xs = [i/1000 for i in timesCwnd]
  ax[0].plot(xs, CWND, 'b', label='CWND')
  ax[0].plot(xs, USED, 'r', label='Used')
  ax[0].plot(xs, FREE, 'g', label='Free')
  ax[0].legend()
  ax[0].set_ylim(0, maxY*1.1)
  ax[0].set_ylabel("CWND (bytes)")
  ax[0].set_xlabel("Time (s)")

  xs = [i/1000 for i in timesAlpha]
  ax[1].plot(xs, ALPHA, 'b-', label='Alpha')
  ax[1].legend()
  ax[1].set_ylim(0, 1.1)
  ax[1].set_ylabel("Alpha")
  ax[1].set_xlabel("Time (s)")

  maxY = max(max(CENTROID_HIGH), max(CENTROID_LOW))
  xs = [i/1000 for i in timesCentroids]
  ax[2].plot(xs, CENTROID_LOW, 'b-', label='C_low')
  ax[2].plot(xs, CENTROID_HIGH, 'r-', label='C_high')
  ax[2].legend()
  ax[2].set_ylim(0, maxY)
  ax[2].set_xlabel("Time (s)")

  plt.savefig('{}/{}_vatic.pdf'.format(dataPath, filePrefix))  


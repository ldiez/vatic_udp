# same as runmm.py but changing the way simulation is being run

import os, sys
import signal
import argparse
import threading
import subprocess as sp
import PlotvaticTrace as pltVatic
from plot import plot_tput_delay

from time import sleep
from utils import *

class Simulation(object):
  
  def __init__(self, args):
    # the trace to use to emulate the link 
    self.trace = args.trace
    self.port = args.port
    self.n_blks = args.n_blocks
    self.pkts_blk = args.pkts_blk
    self.inter_block_time = args.inter_block_time    
    self.pkt_size = args.pkt_size
    # length of buffer in mahimahi
    self.buf_len = args.buf_len
    # value of delay to give to mm-delay
    self.mm_delay = args.mm_delay
    self.savedir=args.dir
    self.send_rate=args.send_rate
    self.send_time=args.send_time

  def run(self):

    # make output directories
    if not os.path.exists(self.savedir):
      os.makedirs(self.savedir)
    print('Saving all output to this directory: "{}"'.format(self.savedir))

    # fixing dest file names
    saveprefix = '{}'.format(self.trace)
    if self.n_blks is not None:
      saveprefix += '-N_{}'.format(self.n_blks)
    if self.buf_len is not None:
      saveprefix += '-Q_{:.0f}'.format(self.buf_len)
    if self.mm_delay is not None and self.mm_delay > 0:
      saveprefix += '-delay_{:02d}'.format(self.mm_delay)
    if self.send_rate is not None:
      saveprefix += '-rate_{:02d}'.format(self.send_rate)      

    

    print('Results name/preffix: "{}"'.format(saveprefix))
    savepathprefix = os.path.join(self.savedir, saveprefix)

    # starting the simulations
    
    server_ip_for_client = '100.64.0.2'
    if self.mm_delay is not None: 
    #   print('Using mm_delay !!')
      server_ip_for_client = '100.64.0.4'
    client_ip_for_server = '100.64.0.1'
    # server_ip_for_client = '127.0.0.1'
    # client_ip_for_server = '127.0.0.1'

    # (i) receiver command
    receiver_cmd = 'vatic/src/client_program --send_addr={} --log_name={} &'.format(server_ip_for_client, saveprefix)

    # (ii) mm command
    tracepath = os.path.join('traces', 'channels', self.trace)
    mm_cmd = 'mm-link {0} {0} --uplink-log {1}_uplink.csv'.format(tracepath, savepathprefix)
    if self.mm_delay is not None and self.mm_delay > 0:
      mm_cmd = 'mm-delay {} '.format(self.mm_delay) + mm_cmd

    if self.buf_len is not None:
      mm_cmd += ' --uplink-queue=droptail --uplink-queue-args=bytes={}'.format(self.buf_len)
      mm_cmd += ' --downlink-queue=droptail --downlink-queue-args=bytes={}'.format(self.buf_len)

    # (iii) sender command
    sender_cmd = 'vatic/src/server_program --send_addr={} --log_name={}'.format(client_ip_for_server, saveprefix)
    if self.pkts_blk:
      sender_cmd += ' --num_pkts_block={}'.format(self.pkts_blk)
    if self.n_blks:
      sender_cmd += ' --num_blocks={}'.format(self.n_blks)      
    if self.inter_block_time:
      sender_cmd += ' --inter_block_time={}'.format(self.inter_block_time)        
    if self.pkt_size:
      sender_cmd += ' --pkt_size={}'.format(self.pkt_size)              
    if self.send_rate:
      sender_cmd += ' --send_rate={}'.format(self.send_rate)                
    if self.send_time:
      sender_cmd += ' --send_time={}'.format(self.send_time)                  

    print('Run Mahimahi with command: {}'.format(mm_cmd))
    sender_process = sp.Popen(mm_cmd, stdin=sp.PIPE, stdout=sp.PIPE, shell=True, universal_newlines=True)
    sleep(2)
    os.system('sudo ip route add 100.64.0.0/24 via 100.64.0.1')
    sleep(2)
    print('Run receiver with command: {}'.format(receiver_cmd))
    os.system(receiver_cmd)
    sleep(2)
    print('Run sender with command: {}'.format(sender_cmd))
    sender_process.communicate(sender_cmd)

    os.system('sudo killall server_program')
    os.system('sudo killall client_program')
    os.system('sudo killall mm-delay')
    os.system('sudo killall mm-link')
    os.system('ps | pgrep -f server_program | xargs kill -TERM')
    os.system('ps | pgrep -f client_program | xargs kill -TERM')
    os.system('ps | pgrep -f mm-delay | sudo xargs kill -TERM')
    os.system('ps | pgrep -f mm-link | sudo xargs kill -TERM')

    baseTime = pltVatic.GetBaseTime('{}_uplink.csv'.format(savepathprefix))
    pltVatic.PlotVatic('results', saveprefix, baseTime)
    plot_tput_delay ('{}_uplink.csv'.format(savepathprefix), save=True, skip_seconds=6, ms_per_bin=10)
    plot_tput_delay ('{}_uplink.csv'.format(savepathprefix), save=True, skip_seconds=6, ms_per_bin=100)
    plot_tput_delay ('{}_uplink.csv'.format(savepathprefix), save=True, skip_seconds=6, ms_per_bin=500)
    plot_tput_delay ('{}_uplink.csv'.format(savepathprefix), save=True, skip_seconds=6, ms_per_bin=1000)

    return

if __name__ == '__main__':
  ''' Interactive program to run a full simulation. '''
  parser = argparse.ArgumentParser(description="Run mahimahi for cellular network simulations")
  parser.add_argument('trace', help='Cellsim traces to be used (channel simulation; choose one from traces/channels/)')
  parser.add_argument('--port', type=int, help='Port number to use')

  parser.add_argument('--n-blocks', help='Number of blocks to send')
  parser.add_argument('--pkts-blk', type=int, help='Size of a block in number of packets')
  parser.add_argument('--inter-block-time', type=int, help='Time in ms to wait to start sending a new block')
  parser.add_argument('--pkt-size', type=int, help='Time in ms to wait to start sending a new block')
  parser.add_argument('--send-time', type=int, help='Sending time in seconds')
  parser.add_argument('--send-rate', type=int, help='Sending rate in Kbps (1e3 bps)')

  parser.add_argument('--buf-len', type=positive_int, help='Buffer size in mahimahi (bytes)')
  parser.add_argument('--mm-delay', type=unsigned_int, help='Delay for mahimahi delay shell (mm-delay)')

  # runtime options
  parser.add_argument('--dir', help='Directory to store outputs', default='results')

  args = parser.parse_args()
  prettyprint_args(args)
  sim = Simulation(args)
  sim.run()

  print("Finished")
# batch running of experiments

import os
import getpass
import argparse
from utils import *
from runmm2 import Simulation

def CleanState (): 
  os.system('sudo killall python2 receiver sender mm-link mm-delay >/dev/null 2>&1 ')
  # os.system('sudo ../ccp-kernel/ccp_kernel_unload')
  # os.system('sudo ../ccp-kernel/ccp_kernel_load ipc=0')
  os.system('sudo sysctl -w net.ipv4.ip_forward=1 >/dev/null 2>&1')
  os.system('sleep 5')

def RunCCP (): 
  CleanState ()
  os.system('sudo nohup python2 ../ccp_bicdctcp/python/bic_dctcp_centroids.py 10000000 -P exp -A 0.5 >./nohup_vatic.log 2>&1 &')
  os.system('sleep 5')

if __name__ == '__main__':
    

    parser = argparse.ArgumentParser()
    megroup = parser.add_mutually_exclusive_group()
    parser.add_argument('--savedir', help='Directory to store outputs', default='results')
    megroup.add_argument('--ttr', '-t', type=positive_int, metavar='DURATION_SEC', 
                         help='Duration to run the experiment (seconds)', default=30)
    megroup.add_argument('--n-blocks', '-n', type=positive_int, metavar='NUM_BLOCKS',
                         help='Number of blocks to send')
    megroup.add_argument('--trace-file', '-f', dest='filepath',
                         help='Trace file from which to generate traffic for sender')
    parser.add_argument('--blksize', '-b', type=int,
                        help='Size of a block (multiples of KiB)',
                        default=Simulation.BLKSIZE_DEFAULT)
    parser.add_argument('--cc-algo', '-C', choices=('cubic', 'reno', 'bbr', 'ccp', 'ledbat', 'verus', 'pcc'), help='CC algo to use',
    default='bbr')
    # parser.add_argument('--mm-delay', type=unsigned_int,
    #                     help='Delay for mahimahi delay shell (mm-delay)')
    parser.add_argument('--log', '-l', action='store_true',
                        help='Log packets at the sender and receiver',
                        default=False)
    parser.add_argument('--skip-seconds', type=unsigned_int, 
                        help='Skip initial seconds before computing performance (default: 0)',
                        default=0)
    parser.add_argument('--verbose', '-v', action='count',
                        help='Show verbose output',
                        default=0)
    args = parser.parse_args()
    prettyprint_args(args)
    port = 9999

    # trace_list = ['Building_a_new', 'Building_b_new', 'Building_c_new', 'InHM_a_new', 'InHM_b_new', 'InHM_c_new']
    # maxbw_list = [, 1000, 700, 1015, 1015, 1030]# Megabits/sec
    trace_list = ['LOW_10-HIGH_100']
    maxbw_list = [200]# Megabits/sec
    mmdelay_list = [5]
    nrep = 1

    for trace, maxbw in zip(trace_list, maxbw_list):
        #trace += '_new'
        print(os.linesep + '**** Trace: {}, Max BW: {} Mbps ****'.format(trace, maxbw) + os.linesep)
        # exit(-1)
        for delay in mmdelay_list:
            bdp = (maxbw * delay) * (1e3 / 8)
            bdpfactor_list = [None] #[None,1,2,4]
            for bdpfactor in bdpfactor_list:
                if bdpfactor is None:
                    qsize=None
                    print('#### mm-delay DELAY = {} ms, buffer size = Infinite ####'.format(delay) + os.linesep)
                else:
                    qsize = bdpfactor * bdp
                    qsize_nice, unit_nice = get_bytes_nice(qsize, kibi=False)
                    print('#### mm-delay DELAY = {} ms, buffer size = {}*BDP = {} {} ####'.format(delay, bdpfactor, qsize_nice, unit_nice) + os.linesep)
                sim = Simulation(trace, port, args.ttr, args.n_blocks, args.filepath, args.blksize, args.cc_algo, qsize, delay)
                for i in range(1, nrep+1):
                    #sim.run(root_passwd, args.savedir, args.log, args.skip_seconds, args.verbose, suffix=str(i))
                    try: 
                      RunCCP ()
                      # print ('Running !!!')
                      # sim.run(args.savedir+"_{:02d}".format(i), args.log, args.skip_seconds, args.verbose)
                      sim.run(args.savedir, args.log, args.skip_seconds, args.verbose)
                    except: 
                      print('Something failed in scenario {}... continue'.format(trace))  

#okPlease wait for 5 mins
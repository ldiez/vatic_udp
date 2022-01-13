# VATIC on UDP

Provisional repository with UDP-based VATIC implementation. It is based on [mmwave_cc](https://github.com/shivariyer/mmwave_cc) repo.

## Requirements

Python 3, NumPy, Pandas, Matplotlib, Make,
[Mahimahi](mahimahi.mit.edu), Boost libraries, C++ 17 compatible compiler (tested with gcc/g++ 9.3), auto-tools.

## Building

### Build the Vatic client and server:

The code of the Vatic implementation is in the _vatic_ folder.

- For the first compilation you need to build the project (autotools)

```
cd vatic
autoreconf --install
./configure
make
```

- Once built, just use the _make_ command

### Other CC protocols

The other protocols are in the root folder; just build the client and server programs inside `sender_receiver` by
typing `make`

## Execution

### Vatic

Run the script 'runmmVatic.py', for instance:

```console
python3 runmmVatic.py LOW_10-HIGH_100 --send-time=30
```

### Other protocols

Run the script 'script_runmm2.py', as follows:

```console
python3 script_runmm2.py --savedir=results_bbr --cc-algo=bbr
```

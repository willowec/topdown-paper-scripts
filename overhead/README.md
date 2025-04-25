# Overhead Tests for PAPI Topdown Component

This README describes the process for testing the cycle overhead of four different
methods for collecting topdown metrics in PAPI.

## Requirements

- An 10th+ generation x86 Intel processor running Linux
- Privilages to install/uninstall software

## Setup

1. Download PAPI source with the topdown component patch

	1. Clone PAPI from willowec: `git clone https://github.com/willowec/papi-devel.git`
	2. Checkout the branch with the topdown component: `git switch topdown-rdpmc-protected`

2. Download librseq source with a patch removing thread-local storage

	1. Clone librseq from willowec: `https://github.com/willowec/librseq-dev.git`
	2. Checkout the branch with thread-local storage disabled: `git switch dlsym-compat`

3. In this directory, run `make` to compile the overhead tests

## Procedure

1. Test using perf_event component with rdpmc enabled:
	1. configure and install PAPI for test 1 (est. time ~1 min):

		in `papi/src/` run `./configure && make && sudo make install`

	2. run the overhead test (est. time ~3 mins)

		in this directory, run `taskset -c 0 ./overhead_rdpmc`

	3. note down the cycle count results (min, q1, median, q3, max)

2. Test using perf_event component with rdpmc *disabled*:
	1. configure and install PAPI for test 1 (est. time ~1 min):

		in `papi/src/` run `./configure --disable-perfevent_rdpmc && make && sudo make install`

	2. run the overhead test (est. time ~7 mins)

		in this directory, run `taskset -c 0 ./overhead_disrdpmc`

	3. note down the cycle count results (min, q1, median, q3, max)

3. Test using topdown component without librseq:
	1. configure and install PAPI for test 1 (est. time ~1 min):

		in `papi/src/` run `./configure --with-components=topdown && make && sudo make install`

	2. ensure librseq is not installed

		in `librseq` run `sudo make uninstall`

	3. run the overhead test (est. time ~2 mins)

		in this directory, run `taskset -c 0 ./overhead_td`

	4. note down the cycle count results (min, q1, median, q3, max)

4. Test using topdown component *with* librseq:
	1. use the same PAPI configuration as last time (no need to reinstall)

	2. ensure librseq *is* installed

		in `librseq` run `sudo make install`

	3. run the overhead test (est. time ~2 mins)

		in this directory, run `taskset -c 0 ./overhead_td`

	4. note down the cycle count results (min, q1, median, q3, max)

## Measuring PMCs to look for patterns in cycle overhead spikes

The `overhead_test.c` program has the infrastructure laid out to allow for hardware performance counters to be measured via the Linux perf_event interface for the PAPI_start() and PAPI_stop() calls. 
To enable, set `#define PROFILE 1` and `#define PROF_EVENT "some event"`

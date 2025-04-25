# Heterogeneous context switch robustness test for PAPI topdown component

This directory contains scripts for testing the efficacy of the guard rails implemented in the `topdown` component to ensure segmentation faults are never raised due to an attempt to access an unsupported hardware performance counter after the program has been migrated to an unsupported core.

## Setup

1. configure and install PAPI with the `topdown` component enabled, with librseq either enabled or disabled
2. `make`
3. `./run_tests.sh <core_start> <core_end> <n_tests> <output_file>`

## Analyzing results

Run `python3 successes.py` to obtain the failure rate

## Note for reproduction

It is imperative that the random time delay in `run_tests.sh` line 25 be configured for your machine to ensure that the context switch always occurs after the ctx_switch program has reached its main loop, and before the ctx_switch program exits its main loop.
The delay was selected by manual tuning for the purposes of the paper

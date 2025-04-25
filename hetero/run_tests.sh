#!/bin/bash

# usage: ./run_tests <start_core> <end_core> <N>
# ex: ./run_tests 0 17 100000

echo starting from $1 to $2, $3 times
sleep 1

mkdir $4

# warm up processor / load up PAPI so that papi prep time is 0ms for loop runs
taskset -c 0 ./ctx_switch &>/dev/null
taskset -c 0 ./ctx_switch &>/dev/null
taskset -c 0 ./ctx_switch &>/dev/null
taskset -c 0 ./ctx_switch &>/dev/null

# repeatedly start the program, wait a random time that is guaranteed to occur
# within the start/stop loop, and move to an unsupported core
for i in $(seq 1 $3);
do
	taskset -c $1 ./ctx_switch &
	PID=$!

	# prog takes 0ms to load papi, ~35ms to complete loop. wait 10.0-20.0ms
	TIME=$( printf "0.0%03d" $(shuf -i 100-200 -n 1) )
	#echo $TIME s
	sleep $TIME

	# now shift to an unsupported core
	taskset -cp $2 $PID
	wait $PID

	echo $? >> ${4}/${3}_${1}to${2}
done

echo saved results into $4
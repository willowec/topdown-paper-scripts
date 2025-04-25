from pathlib import Path
import argparse


segfaults = 0
noerr = 0
total = 0

parser = argparse.ArgumentParser()
parser.add_argument("datafile", type=Path, help="the results file to read")
args = parser.parse_args()

with open(args.datafile, 'r') as f:
	for l in f.readlines():
		total += 1

		if l.rstrip() == '139':
			segfaults += 1
		if l.rstrip() == '0':
			noerr += 0


fail_rate = segfaults / total * 100
print(total, "runs.")
print("Program completed", noerr, "times. (should be 0)")
print("Segfaults:", segfaults, f"\nFailure Rate: {fail_rate}%")

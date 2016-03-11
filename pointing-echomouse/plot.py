#!/usr/bin/env python
import pylab as plt
import os
import numpy as np
import re
import argparse
from urlparse import urlparse, parse_qs

parser = argparse.ArgumentParser(description='Plot the transfer function.')
parser.add_argument("-d", "--dir", dest="dirname", required=True,
                  help="Read directory containing config dict", metavar="DIRECTORY")

args = parser.parse_args()
dirname = args.dirname

filename = os.path.join(dirname, "config.dict")

def parse_dict(dict_file):
	result = {}
	with open(dict_file) as data_file:
		for line in data_file:
		    if line[0] != '#' and ':' in line:
		    	key, value = line.split(':', 1)
		    	result[key.strip()] = value.strip()
	return result

confDict = parse_dict(filename)

parsedInput = parse_qs(urlparse(confDict['libpointing-input']).query)
#parsedOutput = parse_qs(urlparse(confDict['libpointing-output']).query)

ipm = 0.0254
system = confDict["system"]
display_res = 110.5
point_res = float(parsedInput["cpi"][0])
ihz = float(parsedInput["hz"][0])

def natural_sort(l):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
    return sorted(l, key = alphanum_key)

def mouse_counts_to_meters(counts):
	return float(counts) / point_res * ipm * ihz

def display_counts_to_meters(counts):
	return float(counts) / display_res * ipm * ihz


f1 = plt.figure(1)
f2 = plt.figure(2)
ax1 = f1.add_subplot(111)
ax2 = f2.add_subplot(111)

funcs = confDict['functions'].split(',')
func_aliases = confDict['function-aliases'].split(',')

for i, f in enumerate(funcs):
	name = f + ".dat"
	filename = os.path.join(dirname, name)
	fileDict = parse_dict(filename)
	keys = [key for key in fileDict if key.isdigit()]

	mps = []
	mds = []

	for key in natural_sort(keys):
		value = fileDict[key]
		mps.append(mouse_counts_to_meters(key))
		mds.append(display_counts_to_meters(value))

	mps = np.array(mps)
	mds = np.array(mds)

	gains = mds / mps
	#print gains
	#print(gains)
	ax1.plot(mps, gains, label=str(func_aliases[i]))
	ax2.plot(mps, mds, label=str(func_aliases[i]))

plots = ("gain", "visual speed (m/s)")
for i, ylabel in enumerate(plots):
	plt.figure(i + 1)
	plt.xlabel("motor speed (m/s)")
	plt.ylabel(ylabel)
	plt.title(system)
	plt.legend(loc="best")

plt.show()
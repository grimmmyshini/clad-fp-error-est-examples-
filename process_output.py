#!/usr/bin/env python

import json
import sys

f = open(sys.argv[1])
memory_usage = sys.argv[2]

data = json.load(f)

i = 0
name, inp = data['benchmarks'][i]['name'].split("/", 1)
time = data['benchmarks'][i]['real_time']
mbu = data['benchmarks'][i].get('max_bytes_used', -1)

print('{:>25}, {:>10}, {:15.2f}, {:20d}, {:>20}'.format(name, inp, time, mbu, memory_usage))
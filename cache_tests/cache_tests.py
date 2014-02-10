#! /usr/bin/python

import os
import re
import shutil
import subprocess

MAX_SIZE = '20000'
NB_READS = '10000000'

cmd = ['./cache_tests', MAX_SIZE, NB_READS]
p = re.compile('[0-9]+$')
out = subprocess.Popen(cmd + ['seq'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
_data_seq_ = ''
for line in out.split('\n'):
    if p.match("".join(line.split()).replace('.', '')):
        splits = line.split()
        _data_seq_ += splits[0] + "," + splits[1] + '\n'
    elif line.startswith('L1 = '):
        _L1_ = line.split("=")[1]
    elif line.startswith('L2 = '):
        _L2_ = line.split("=")[1]
    elif line.startswith('L3 = '):
        _L3_ = line.split("=")[1]

out = subprocess.Popen(cmd + ['rand'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
_data_rand_ = ''
for line in out.split('\n'):
    if p.match("".join(line.split()).replace('.', '')):
        splits = line.split()
        _data_rand_ += splits[0] + "," + splits[1] + '\n'

# Replace in template file
if os.path.exists('results'):
    shutil.rmtree('results')
os.makedirs('results')
with open("results/results.tex", 'w+') as texFile:
    with open("results.tmpl.tex", "rt") as texTmplFile:
        for line in texTmplFile:
            line = line.replace('_data_seq_', _data_seq_)
            line = line.replace('_data_rand_', _data_rand_)
            line = line.replace('_L1_', str(int(_L1_) / 1024))
            line = line.replace('_L2_', str(int(_L2_) / 1024))
            line = line.replace('_L3_', str(int(_L3_) / 1024))
            texFile.write(line)
subprocess.call(['pdflatex', 'results.tex'], cwd='results', stdout=subprocess.PIPE, stderr=subprocess.PIPE)

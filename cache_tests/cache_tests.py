#! /usr/bin/python

import os
import re
import shutil
import subprocess

p = re.compile('[0-9]+$')
out = subprocess.Popen('./cache_tests', stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
_data_ = ''
for line in out.split('\n'):
    if p.match("".join(line.split())):
        splits = line.split()
        _data_ += splits[0] + "," + splits[1] + '\n'

# Replace in template file
if os.path.exists('results'):
    shutil.rmtree('results')
os.makedirs('results')
with open("results/results.tex", 'w+') as texFile:
    with open("results.tmpl.tex", "rt") as texTmplFile:
        for line in texTmplFile:
            line = line.replace('_data_', _data_)
            texFile.write(line)
subprocess.call(['pdflatex', 'results.tex'], cwd='results', stdout=subprocess.PIPE, stderr=subprocess.PIPE)

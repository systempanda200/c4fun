#! /usr/bin/python

import os
import shutil
import subprocess

FREQS = [10000, 5000, 4000, 2000, 1000, 500, 100]
#FREQS = [10000]
MEM_SIZES = [2, 4, 8, 16, 32, 64, 128, 256]
#MEM_SIZES = [2, 4, 8, 16, 32]

MODE_SEQ = 'seq'
MODE_RAND = 'rand'
MODES = [MODE_SEQ, MODE_RAND]

class Bench_Run:

    def __init__(self, size, freq, mode, nb_samples, nb_memory_samples):
        self.size = size
        self.freq = freq
        self.mode = mode
        self.nb_samples = nb_samples
        self.nb_memory_samples = nb_memory_samples
    def get_mem_ratio(self):
        if self.nb_samples == 0:
            return 0;
        else:
            return self.nb_memory_samples / float(self.nb_samples) * 100

runs = []
for size in MEM_SIZES:
    for freq in FREQS:
        for mode in MODES:
             cmd = ['pebs_bench', str(size), mode, str(freq)]
             print('running ' + str(cmd) + ' ...')
             out = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
             for line in out.split('\n'):
                 if line.find('memory samples on') != -1:
                     splits = line.split(' ')
                     run = Bench_Run(size, freq, mode, long(splits[4]), long(splits[0]))
                     runs.append(run)
                     break

seq_data = ''
rand_data = ''
size = 0;
for run in runs:
    if run.size != size:
        if size != 0:
            seq_data += '\n'
            rand_data += '\n'
        size = run.size
        seq_data += str(size)
        rand_data += str(size)
    if run.mode == MODE_SEQ:
        seq_data += ',' + str(run.get_mem_ratio())
    else:
        rand_data += ',' + str(run.get_mem_ratio())

seq_plot = ''
seq_legend = '\t\\legend{'
rand_plot = ''
rand_legend = '\t\\legend{'
i = 0;
for freq in FREQS:
    if i > 0:
        seq_plot += '\n'
        seq_legend += ', '
        rand_plot += '\n'
        rand_legend += ', '
    seq_plot += '\t\\addplot table[x index=0,y index=' + str(i + 1) + ',col sep=comma] {seq.dat};'
    seq_legend += str(freq)
    rand_plot += '\t\\addplot table[x index=0,y index=' + str(i + 1) + ',col sep=comma] {rand.dat};'
    rand_legend += str(freq)
    i += 1
seq_legend += '}'
rand_legend += '}'

# Replace in template file
if os.path.exists('results'):
    shutil.rmtree('results')
os.makedirs('results')
with open("results/results.tex", 'w+') as texFile:
    with open("results.tmpl.tex", "rt") as texTmplFile:
        for line in texTmplFile:
            line = line.replace('seq_data', seq_data)
            line = line.replace('seq_plot', seq_plot)
            line = line.replace('seq_legend', seq_legend)
            line = line.replace('rand_data', rand_data)
            line = line.replace('rand_plot', rand_plot)
            line = line.replace('rand_legend', rand_legend)
            texFile.write(line)
subprocess.call(['pdflatex', 'results.tex'], cwd='results', stdout=subprocess.PIPE, stderr=subprocess.PIPE)

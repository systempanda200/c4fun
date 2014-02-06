c4fun
=============

This project contains several C independent programs and some common
libraries for Linux used to illustrate some architectural details of
the hardware executing the programs.

To compile programs, just run make from the project's root directory.

The different programs are:

* **cache_tests:** Determine through timing memory accesses cache
    levels and sizes.

* **mem_alloc:** Library used by other programs to allocate and fill
    memory ready for pointer chasing. Each memroy "cell" points to
    another memory cell in the memory region. The library provide
    sequential memory filling and pseudo-random memory filling.

* **pebs_tests:** For Intel Nehalem processors only. Benchmark
    illustrating the PEBS (Precise Event Based Sampling) load latency
    feature provied by Intel Nehalem's PMU (Performance Monitoring
    Unit) hardware.

* **perf_event_open_tests:** Simple example of how using the Linux
    perf_event_open system call providing an abstraction of underlying
    PMU.
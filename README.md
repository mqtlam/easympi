easympi
=======

Easily add parallel processing to your C++ applications.

This is intended for master-slave architectures where the slaves process certain tasks in parallel while the master is responsible for merging those results and performing practically the rest of the computations. There are two routines: 1) master process waits until all slaves arrive at that point and 2) any slave process waits until the master arrives at that point.

Please see Demo.cpp for a Hello World style program.

Improvements and corrections are welcomed.

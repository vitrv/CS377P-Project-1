CS 377P: Programming For Performance
Assignment 1: Performance Counters
Laura Catano, UT EID: lnc652
George Ellis, UT EID: ghe88

============================

HOW TO RUN THIS PROGRAM:

1. Set up the environment by running 'export PATH=$PATH:/opt/intel/bin'
2. Run 'make'
3. Run the program by running './count'

============================

OUTPUT:

For each size and iteration, from a 50 x 50 matrix to a 2000 x 2000 matrix, the program
outputs the following information after multiplication:

Example:

		=== Size: 400 Order: K-J-I ===
Total cycles: 1849804600
Total instructions: 466083221
Total load/store instructions: 384162089
Total floating point instructions: 128121255
L1 data cache accesses: 485810912
L1 data cache misses: 152540560
L2 data cache accesses: 152540560
L2 data cache misses: 8070247
Thread time: 0.416554122 seconds
Real time: 0.417677282 seconds

NOTE: This data is also output in csv format as 'output.csv'

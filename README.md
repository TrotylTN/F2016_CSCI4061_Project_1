# CSCI4061_Project_1
Project #1 of CSCI 4061, which will due at Oct 5 midnight. 

Sep. 23, 2016:

7.50pm: 

Completed DAG analysis, building processing queue and "make4061 -n" function. by Tiannan.

Sep. 24, 2016:

5.55pm: 

1. Replaced the queue with a processing matrix in order to parallel-run commands in further development. Now our project can divided the targets into correct corresponding running sequences.

2. Re-organized the algorithm for DAG parse.

3. Put timestamp comparing struct into DAG parse. Now our project can determine whether a target should be re-compile. Completed 'make4061 -n' function entirely.

4. Completed methods for 'make4061 -B'.
# mpi-bw-latency-tests

## Description

OpenMPI (v1.5) bandwith and latency tests:

* 1: Process ``X`` sends to process ``Y`` bursts of ``N`` packets whose size is ``M`` bytes.
* 2: Process ``Y`` receives them and then sends to process ``X`` an integer as acknowledgement.
* 3: Go to 1 until ``R`` repetitions of the test are done.
* 4: Measures bandwith, mean latency, worst latency and best latency.

Number of pairs of processes doing the test is parametrized, as well as ``N`` (number of packets), ``M`` (packet size) and ``R`` (number of repetitions).

Multiple implementations of the test:
* One communicator for each pair of processes.
* One communicator for all processes, everyone uses same tag to communicate with its assigned mate.
* One communicator for all processes, each pair of processes uses a unique tag.

## Author

* [Agustín Navarro Torres](https://github.com/SirBargus)
* [Marcos Canales Mayo](https://github.com/MarcosCM)
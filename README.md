In the intelligent transportation system, the calculation of the shortest path
and the best path is an important link of the vehicle navigation procedure. Due to more and
more real-time information to participate in the calculation, the calculation requires high
efficiency of the algorithm. One of the many common algorithms in solving the shortest
path problem is Dijkstra's algorithm. The algorithm has its advantages on both reducing the
number of repeated operations and reading the shortest path and the path length from the
startpoint to all the other nodes by the shortest path tree or by the feature matrix. In this
paper, we show that the parallel implementation of Djikstra’s algorithm, which is based on
the message passing interface, provides better efficiency and better speedup when compared
to the algorithm’s sequential execution.
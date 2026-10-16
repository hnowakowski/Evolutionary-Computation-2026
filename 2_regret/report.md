# Greedy regret heuristics

### Hubert Nowakowski 160302

### Mukhammad Sattorov 159351

### Code link: https://github.com/hnowakowski/Evolutionary-Computation-2026/blob/main/2_regret/main.cpp

## Description of the problem

We're trying to find a hamiltonian cycle where:

- function to be minimized is defined as distance + cost
- we want to have the cycle consist of exactly 50% of nodes
- distances between nodes are rounded euclidean

All solutions are described as a vector of subsequent node indices, their objectives were tested using the solution checker excel sheet

## Approaches and pseudocode

### Nearest neighbour 2-regret

On each iteration, we pick the two nearest nodes from all the possible insertion points. Then, we add the one where the difference between the best and the second best option is smallest. At the end we create a direct connection from the end node to the start.

Time complexity: O(n^3)

```
put first node into solution
create vector with a list of all used nodes
add first node to used nodes list
while solution size < desired length:
    create vector of insertion costs for each node
    for j from 0 to solution length + 1:
        for k from 0 to matrix length:
            if index k is true in the used vector, continue
            else if j == 0:
                if matrix[k][solution[j]] < smallest objective so far:
                 set as current choice

            else if j == solution length and matrix[solution[j-1]][k] < smallest objective so far, set as current choice
            else if -matrix[solution[j-1]][solution[j]] + matrix[solution[j-1]][k] + matrix[k][solution[j]] < smallest objective so far, set as current choice
    insert best node at best position
    add node to the pool
return solution
```

TSPA:<br>
Max objective: 79954<br>
Average objective: 76092<br>
Min objective: 73415<br>

TSPB:<br>
Max objective: 67963<br>
Average objective: 61692<br>
Min objective: 55523<br>

![](./nearestend.png)

### Greedy cycle 2-regret

On each iteration, we pick the nearest (lowest objective function increase) node from all possible node insertion points. At the end we create a direct connection from the end node to the start.

Time complexity: O(n^3)

```
put first node into solution
create vector with a pool of available nodes (all aside from the initial node)
while solution size < desired length:
    for each j from 0 to solution length + 1:
        for each k node that exists:
            if the node k is not in the pool, continue
            else if j == 0 and matrix[k][solution[j]] < smallest objective so far, set as current choice
            else if j == solution length and matrix[solution[j-1]][k] < smallest objective so far, set as current choice
            else if -matrix[solution[j-1]][solution[j]] + matrix[solution[j-1]][k] + matrix[k][solution[j]] < smallest objective so far, set as current choice
    insert best node at best position
    add node to the pool
return solution
```

TSPA:<br>
Max objective: 78761<br>
Average objective: 75726<br>
Min objective: 73566<br>

TSPB:<br>
Max objective: 67407<br>
Average objective: 61667<br>
Min objective: 55417<br>

![](./nearestall.png)

### Nearest neighbour 2-regret weighted

On each iteration, expand our current cycle by adding the nearest node from any possible node insertion points. Very similar to previous approach, except we constantly treat the vector as a cycle so at the end we don't need to add a connection to the beginning.

Time complexity: O(n^3)

```
put first node into solution
create vector with a pool of available nodes (all aside from the initial node)
while solution size < desired length:
    for each j from 0 to solution length + 1:
        for each k node that exists:
            if the node k is not in the pool, continue
            if j == 0 and -matrix[solution[solution size - 1]][solution[j]] + matrix[solution[solution size - 1]][k] + matrix[k][solution[j]] < smallest objective so far, set as current choice
            else if -matrix[solution[j-1]][solution[j]] + matrix[solution[j-1]][k] + matrix[k][solution[j]] < smallest objective so far, set as current choice
    insert best node at best position
    add node to the pool
return solution
```

TSPA:<br>
Max objective: 77573<br>
Average objective: 75154<br>
Min objective: 72913<br>

TSPB:<br>
Max objective: 68359<br>
Average objective: 60725<br>
Min objective: 54978<br>

![](./greedycycle.png)

### Greedy cycle 2-regret weighted

On each iteration, expand our current cycle by adding the nearest node from any possible node insertion points. Very similar to previous approach, except we constantly treat the vector as a cycle so at the end we don't need to add a connection to the beginning.

Time complexity: O(n^3)

```
put first node into solution
create vector with a pool of available nodes (all aside from the initial node)
while solution size < desired length:
    for each j from 0 to solution length + 1:
        for each k node that exists:
            if the node k is not in the pool, continue
            if j == 0 and -matrix[solution[solution size - 1]][solution[j]] + matrix[solution[solution size - 1]][k] + matrix[k][solution[j]] < smallest objective so far, set as current choice
            else if -matrix[solution[j-1]][solution[j]] + matrix[solution[j-1]][k] + matrix[k][solution[j]] < smallest objective so far, set as current choice
    insert best node at best position
    add node to the pool
return solution
```

TSPA:<br>
Max objective: 76264<br>
Average objective: 74584<br>
Min objective: 72863<br>

TSPB:<br>
Max objective: 67335<br>
Average objective: 60143<br>
Min objective: 54231<br>

Best solution out of 200 runs:

![](./greedycycle.png)

## Conclusions

While the random approach is well...random, greedy approaches present a tradeoff dynamic. If we consider only the nearest neighbour of the last node, we have to do much less computation, but will miss out on more optimal solutions that the all possibilities variant explores. While the processing time is MUCH longer, it does yield substantially better scores on our test dataset. And, as a further refinement, we can also take care of the issue with a horrendously unoptimal final edge by treating the solution as a loop throughout the algorithm with the greedy cycle method.

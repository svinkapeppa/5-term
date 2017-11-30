# Fourth task
&emsp;task.pdf - formulated task  
&emsp;graph.h - header file  
&emsp;graph.c - graph implementation  
&emsp;random.h - header file  
&emsp;random.c - implementation of the special functions for the thread which manages random numbers  
&emsp;thread_pool.h - header file  
&emsp;thread_pool.c - thread pool implementation  
&emsp;genetic.h - header file  
&emsp;genetic.c - implementation of the genetic part  
&emsp;create.c - small function to create graph  
&emsp;main.c - main program which starts everything
&emsp;plots.ipynb - plots which were asked in the task

## Crossover
Crossover works as follows: we select two individuals, select some verticies in the former one and then complete it with the remaining vertices taken in the order in which they are located in the latter one.

### Mutation
Mutation works as follows: we simply swap two verticies in the route.

#### Selection
Selection works as follows: from the two random individuals we select the one, whose fitness function value is better.


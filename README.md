# Genetic_Algorithm

Purpose of the project: Implementation of a program that operates in distributed environment searching for the minimum number of overlapping non-coinciding convex polygons covering any polygon.

Whole programm is written in C++ and C, for connecting master-role computer with slave-role computers, I have used PVM environment. Istances that were used for testing were setuped in AWS. All the instances' OS is Ubuntu 22.04.3 LTS.

[Genetic_Algorithm/main.cpp](Genetic_Algorithm/main.cpp) stores base main program that works on one instance without using a distributed environment.

[PVM](PVM) stores cpp files (both master and slave) that use PVM distributed environment. Works with g++ compilter.

Program execution steps:
- **Population Initialization:** The master creates the initial population of polygons and evenly distributes them among nodes.
- **Population Evaluation:** Each node evaluates its local solutions by calculating the number of overlapping polygons.
- **Selection:** Each slave performs the selection operation.
- **Crossover:** Crossover of the population and the creation of offspring are the responsibility of slave nodes. Half of the parents pass half of their vertices to the child.
- **Mutation:** Based on the MUTATION_RATIO, a slave can modify the vertices of the offspring's polygons. Depending on the value of this variable, the chances of mutation increase or decrease.
- **Replacement:** After performing the selection and crossover operations, the slave replaces the main population with the offspring.
- **Transmission of Results:** Alle the slaves send their results to master

Results:
![Screenshot_61](https://github.com/ZasaQ/Genetic_Algorithm/assets/83781535/394e102e-04d2-42f0-ab02-5fd96351c697)

---
layout: default
title: Archive Concept
nav_order: 1
has_children: false
parent: Archive Container
has_toc: false
---
# Archive Concept

It's often useful to cache elements even if they are not in the Pareto front. For instance, in dynamic applications, we might need a replacement
for an element in the front that's not longer available, or only few elements might make it to the front and we might need more options. In these cases, you can use **Pareto archives** to keep track of the elements which are second-best, third-best, ....

A **Pareto archive** $A$ is then a list of Pareto fronts $[P^1, P^2, \dots, P^{|A|}]$ ordered by their ranks. In other words, the archive front $P^{i+1}$ is the Pareto front we would have if we removed $P^i \cup P^{i-1} \cup \dots \cup P^1$ from $F$.

!!! example "Pareto archive"
    A two-dimensional Pareto archive
    
    ![2-dimensional front](../img/archive2d.svg)

    An archive works as if it were a multidimensional stack

!!! summary "Formal Definition: Pareto archive"
    A **Pareto archive** $A$ is a list of Pareto fronts $[P^1, P^2, \dots, P^{|A|}]$ ordered by their ranks. Given the set of feasible solutions $F$, a front $P^i$ is and archive $A$ are defined as

    $$
    \begin{equation}
    \label{eq:archive_front}
    \begin{split}
    A & = \{P^1, P^2, \dots\} \\
    P^i & = \begin{cases} P &\mbox{if } i = 1 \\ \{ x \in F \;|\; \tilde \exists y \in (F \setminus P^{i-1} \setminus P^{i-2} \setminus \dots \setminus P^1) \; y \prec x \} &\mbox{if } i \neq 1 \end{cases}
    \end{split}
    \end{equation}
    $$

The archive interface has all the functions a usual front has: insertion, removal, and querying. Searching operations
identify the proper front for the elements. Functions for indicators and dominance relationships use the first fronts as
reference.

When we insert a new element into the archive, and this element dominates other solutions in the archive, the container
moves the dominated elements to higher fronts efficiently instead of erasing them.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->

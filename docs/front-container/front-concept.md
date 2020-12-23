---
layout: default
title: Front Concept
nav_order: 1
has_children: false
parent: Front Container
has_toc: false
---
# Front Concept

Most lifelike problems involve several conflicting goals. For this reason, the concepts of Pareto fronts and archives have applications that range from economics to engineering. In Game Theory, we have these kinds of outcomes:

|  Outcome        |    Description      |
|-----------------|---------------------|
| **Pareto efficient** or **Pareto optimal** | No other outcome can increase the utility in one goal without decreasing the utility of any other goal |
| **Pareto inefficient** | There is another that can improve at least one goal without harming other goals |
| **Pareto improvement** over $p$ | Better than the Pareto inefficient outcome $p$ |
| **Pareto dominated** by $p$ | Outcome $p$ can improve at least one goal without harming other goals |
| **Pareto dominated** by $p$ | Outcome $p$ can improve at least one goal without harming other goals |

Although many outcomes can be Pareto optimal, no outcome dominates an outcome that is Pareto optimal. The set of all Pareto optimal outcomes is the **Pareto front** (also *Pareto frontier*, or *Pareto set*).

!!! example "Example: Pareto front"
    This is a two-dimensional Pareto front. The region in gray is dominated by the front.
    
    ![2-dimensional front](../img/front2d_b.svg)

    In this example, we consider lower values of $f(x)$ to be a gain of utility

!!! summary "Formal Definition: Pareto front"
    The set $P$ of all Pareto optimal outcomes, is defined as

    $$
    P = \{\; x \;|\; \tilde \exists y\; \exists i\; (f_i(y) < f_i(x)) \;\} = \{\; x \;|\; \tilde \exists y\; (y \prec x)\}
    $$

    where $f_i(x)$ is the $i$-th goal in our problem

Every game has at least one outcome that is Pareto optimal.

The container `pareto::front` is an extension and an adapter of spatial containers for Pareto fronts. The container uses query predicates to find and erase any dominated solution whenever a new solution is inserted.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->

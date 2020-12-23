import pareto

# Constructor
pf = pareto.front(['min', 'max', 'min'])

pf[-2.57664, -1.52034, 0.600798] = 17
pf[-2.14255, -0.518684, -2.92346] = 32
pf[-1.63295, 0.912108, -2.12953] = 36
pf[-0.653036, 0.927688, -0.813932] = 13
pf[-0.508188, 0.871096, -2.25287] = 32
pf[-2.55905, -0.271349, 0.898137] = 6
pf[-2.31613, -0.219302, 0] = 8
pf[-0.639149, 1.89515, 0.858653] = 10
pf[-0.401531, 2.30172, 0.58125] = 39
pf[0.0728106, 1.91877, 0.399664] = 25
pf[-1.09756, 1.33135, 0.569513] = 20
pf[-0.894115, 1.01387, 0.462008] = 11
pf[-1.45049, 1.35763, 0.606019] = 17
pf[0.152711, 1.99514, -0.112665] = 13
pf[-2.3912, 0.395611, 2.78224] = 11
pf[-0.00292544, 1.29632, -0.578346] = 20
pf[0.157424, 2.30954, -1.23614] = 6
pf[0.453686, 1.02632, -2.24833] = 30
pf[0.693712, 1.12267, -1.37375] = 12
pf[1.49101, 3.24052, 0.724771] = 24

if [1.49101, 3.24052, 0.724771] in pf:
    print('Element access:', pf[1.49101, 3.24052, 0.724771])
else:
    print("[1.49101, 3.24052, 0.724771] was dominated")

# Iterators
print('Iterators')
for [point, value] in pf:
    print(point, '->', value)

print('Reversed Iterators')
for [point, value] in reversed(pf):
    print(point, '->', value)

# Capacity and Reference Points
if pf:
    print('Front is not empty')
else:
    print('Front is empty')

print(len(pf), 'elements in the front')
print(pf.dimensions(), 'dimensions')
for i in range(pf.dimensions()):
    if pf.is_minimization(i):
        print('Dimension', i, ' is minimization')
    else:
        print('Dimension', i, ' is maximization')
    print('Best value in dimension', i, ':', pf.ideal(i))
    print('Min value in dimension', i, ':', pf.min_value(i))
    print('Max value in dimension', i, ':', pf.max_value(i))
    print('Best value in dimension', i, ':', pf.ideal(i))
    print('Nadir value in dimension', i, ':', pf.nadir(i))
    print('Worst value in dimension', i, ':', pf.worst(i))

print('Ideal point:', pf.ideal())
print('Nadir point:', pf.nadir())
print('Worst point:', pf.worst())

# Point-point dominance
p1 = pareto.point([0, 0, 0])
p2 = pareto.point([1, 1, 1])
is_minimization = [True, False, True]
print('p1 dominates p2' if p1.dominates(p2, is_minimization) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2, is_minimization) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2, is_minimization) else 'p1 does not non-dominate p2')

# Front-point dominance
print('pf dominates p2' if pf.dominates(p2) else 'pf does not dominate p2')
print('pf strongly dominates p2' if pf.strongly_dominates(p2) else 'pf does not strongly dominate p2')
print('pf non-dominates p2' if pf.non_dominates(p2) else 'pf does not non-dominate p2')
print('pf is partially dominated by p2' if pf.is_partially_dominated_by(p2) else 'pf is not is partially dominated by p2')
print('pf is completely dominated by p2' if pf.is_completely_dominated_by(p2) else 'pf is not is completely dominated by p2')

# Front-front dominance
pf2 = pareto.front(['min', 'max', 'min'])
for [p, v] in pf:
    pf2[pareto.point([p[0] - 1, p[1] + 1, p[2] - 1])] = v

print('pf dominates pf2' if pf.dominates(pf2) else 'pf does not dominate pf2')
print('pf strongly dominates pf2' if pf.strongly_dominates(pf2) else 'pf does not strongly dominate pf2')
print('pf non-dominates pf2' if pf.non_dominates(pf2) else 'pf does not non-dominate pf2')
print('pf is partially dominated by pf2' if pf.is_partially_dominated_by(pf2) else 'pf is not is partially dominated by pf2')
print('pf is completely dominated by pf2' if pf.is_completely_dominated_by(pf2) else 'pf is not is completely dominated by pf2')

# Indicators
# Hypervolume
print('Exact hypervolume:', pf.hypervolume(pf.nadir()))
print('Hypervolume approximation (10000 samples):', pf.hypervolume(10000, pf.nadir()))

# Coverage
print('C-metric:', pf.coverage(pf2))
print('Coverage ratio:', pf.coverage_ratio(pf2))
print('C-metric:', pf2.coverage(pf))
print('Coverage ratio:', pf2.coverage_ratio(pf))

# Convergence
pf_star = pareto.front(['min', 'max', 'min'])
for [p, v] in pf:
    pf_star[p[0] - 1.0, p[1] + 1.0, p[2] - 1.0] = v

print('GD:', pf.gd(pf_star))
print('STDGD:', pf.std_gd(pf_star))
print('IGD:', pf.igd(pf_star))
print('STDGD:', pf.std_igd(pf_star))
print('Hausdorff:', pf.hausdorff(pf_star))
print('IGD+:', pf.igd_plus(pf_star))
print('STDIGD+:', pf.std_igd_plus(pf_star))

# Distribution
print("Uniformity:", pf.uniformity())
print("Average distance:", pf.average_distance())
print("Average nearest distance:", pf.average_nearest_distance(5))
near_origin = next(pf.find_nearest(pareto.point([0.0, 0.0])))
print("Crowding distance:", pf.crowding_distance(near_origin[0]))
print("Average crowding distance:", pf.average_crowding_distance())

# Correlation
print('Direct conflict(0,1):', pf.direct_conflict(0, 1))
print('Normalized direct conflict(0,1):', pf.normalized_direct_conflict(0, 1))
print('Maxmin conflict(0,1):', pf.maxmin_conflict(0, 1))
print('Normalized maxmin conflict(0,1):', pf.normalized_maxmin_conflict(0, 1))
print('Non-parametric conflict(0,1):', pf.conflict(0, 1))
print('Normalized conflict(0,1):', pf.normalized_conflict(0, 1))

print('Direct conflict(1,2):', pf.direct_conflict(1, 2))
print('Normalized direct conflict(1,2):', pf.normalized_direct_conflict(1, 2))
print('Maxmin conflict(1,2):', pf.maxmin_conflict(1, 2))
print('Normalized maxmin conflict(1,2):', pf.normalized_maxmin_conflict(1, 2))
print('Non-parametric conflict(1,2):', pf.conflict(1, 2))
print('Normalized conflict(1,2):', pf.normalized_conflict(1, 2))

# Queries
for [k, v] in pf.find_intersection(pf.ideal(), pareto.point([-2.3912, 0.395611, 2.78224])):
    print(k, "->", v)

for [k, v] in pf.find_within(pf.ideal(), pareto.point([-2.3912, 0.395611, 2.78224])):
    print(k, "->", v)

for [k, v] in pf.find_disjoint(pf.worst(), pareto.point([+0.71, +1.19, +0.98])):
    print(k, "->", v)

for [k, v] in pf.find_nearest(pareto.point([-2.3912, 0.395611, 2.78224]), 2):
    print(k, "->", v)

for [k, v] in pf.find_nearest(pareto.point([2.5, 2.5, 2.5])):
    print(k, "->", v)

for [k, v] in pf.find_dominated(pareto.point([-10, +10, -10])):
    print(k, "->", v)

for i in range(pf.dimensions()):
    print('Ideal element in dimension', i, ': ', pf.ideal_element(i)[0])
    print('Nadir element in dimension', i, ': ', pf.nadir_element(i)[0])
    print('Worst element in dimension', i, ': ', pf.worst_element(i)[0])

# Relational operators
pf3 = pareto.front(pf)
if pf == pf3:
    print('The containers have the same elements')
else:
    if len(pf) != len(pf3):
        print('The containers do not have the same elements')
    else:
        print('The containers might not have the same elements')
        # You need a for loop after here to make sure

pf4 = pareto.front()
for [k, v] in pf:
    pf4[k] = v

if pf == pf4:
    print('The containers have the same elements')
else:
    if len(pf) != len(pf4):
        print('The containers do not have the same elements')
    else:
        print('The containers might not have the same elements')
        # You need a for loop after here to make sure

if pf_star < pf:
    print('pf* dominates pf')
else:
    print('pf* does not dominate pf')

import pareto

# Constructor
capacity = 1000
ar = pareto.archive(capacity, ['min', 'max', 'min'])

ar[-2.57664, -1.52034, 0.600798] = 17
ar[-2.14255, -0.518684, -2.92346] = 32
ar[-1.63295, 0.912108, -2.12953] = 36
ar[-0.653036, 0.927688, -0.813932] = 13
ar[-0.508188, 0.871096, -2.25287] = 32
ar[-2.55905, -0.271349, 0.898137] = 6
ar[-2.31613, -0.219302, 0] = 8
ar[-0.639149, 1.89515, 0.858653] = 10
ar[-0.401531, 2.30172, 0.58125] = 39
ar[0.0728106, 1.91877, 0.399664] = 25
ar[-1.09756, 1.33135, 0.569513] = 20
ar[-0.894115, 1.01387, 0.462008] = 11
ar[-1.45049, 1.35763, 0.606019] = 17
ar[0.152711, 1.99514, -0.112665] = 13
ar[-2.3912, 0.395611, 2.78224] = 11
ar[-0.00292544, 1.29632, -0.578346] = 20
ar[0.157424, 2.30954, -1.23614] = 6
ar[0.453686, 1.02632, -2.24833] = 30
ar[0.693712, 1.12267, -1.37375] = 12
ar[1.49101, 3.24052, 0.724771] = 24

if [1.49101, 3.24052, 0.724771] in ar:
    print('Element access:', ar[1.49101, 3.24052, 0.724771])
else:
    print("[1.49101, 3.24052, 0.724771] was dominated")

# Iterators
print('Iterators')
for [point, value] in ar:
    print(point, '->', value)

print('Reversed Iterators')
for [point, value] in reversed(ar):
    print(point, '->', value)

print('Front Iterators')
for pf in ar.fronts():
    print('Front with', len(pf), 'elements')
    for [k, v] in pf:
        print(k, '->', v)

# Capacity and Reference Points
if ar:
    print('Archive is not empty')
else:
    print('Archive is empty')

print(len(ar), 'elements in the archive')
print(ar.dimensions(), 'dimensions')
for i in range(ar.dimensions()):
    if ar.is_minimization(i):
        print('Dimension', i, ' is minimization')
    else:
        print('Dimension', i, ' is maximization')
    print('Best value in dimension', i, ':', ar.ideal(i))
    print('Min value in dimension', i, ':', ar.min_value(i))
    print('Max value in dimension', i, ':', ar.max_value(i))
    print('Best value in dimension', i, ':', ar.ideal(i))
    print('Nadir value in dimension', i, ':', ar.nadir(i))
    print('Worst value in dimension', i, ':', ar.worst(i))

print('Ideal point:', ar.ideal())
print('Nadir point:', ar.nadir())
print('Worst point:', ar.worst())
print('Capacity:', ar.capacity())
print('Number of fronts:', ar.size_fronts())

# Point-point dominance
p1 = pareto.point([0, 0, 0])
p2 = pareto.point([1, 1, 1])
is_minimization = [True, False, True]
print('p1 dominates p2' if p1.dominates(p2, is_minimization) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2, is_minimization) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2, is_minimization) else 'p1 does not non-dominate p2')

# Archive-point dominance
print('ar dominates p2' if ar.dominates(p2) else 'ar does not dominate p2')
print('ar strongly dominates p2' if ar.strongly_dominates(p2) else 'ar does not strongly dominate p2')
print('ar non-dominates p2' if ar.non_dominates(p2) else 'ar does not non-dominate p2')
print('ar is partially dominated by p2' if ar.is_partially_dominated_by(p2) else 'ar is not is partially dominated by p2')
print('ar is completely dominated by p2' if ar.is_completely_dominated_by(p2) else 'ar is not is completely dominated by p2')

# Archive-archive dominance
ar2 = pareto.archive(['min', 'max', 'min'])
for [p, v] in ar:
    ar2[pareto.point([p[0] - 1, p[1] + 1, p[2] - 1])] = v

print('ar dominates ar2' if ar.dominates(ar2) else 'ar does not dominate ar2')
print('ar strongly dominates ar2' if ar.strongly_dominates(ar2) else 'ar does not strongly dominate ar2')
print('ar non-dominates ar2' if ar.non_dominates(ar2) else 'ar does not non-dominate ar2')
print('ar is partially dominated by ar2' if ar.is_partially_dominated_by(ar2) else 'ar is not is partially dominated by ar2')
print('ar is completely dominated by ar2' if ar.is_completely_dominated_by(ar2) else 'ar is not is completely dominated by ar2')

# Indicators
# Hypervolume
print('Exact hypervolume:', ar.hypervolume(ar.nadir()))
print('Hypervolume approximation (10000 samples):', ar.hypervolume(10000, ar.nadir()))

# Coverage
print('C-metric:', ar.coverage(ar2))
print('Coverage ratio:', ar.coverage_ratio(ar2))
print('C-metric:', ar2.coverage(ar))
print('Coverage ratio:', ar2.coverage_ratio(ar))

# Convergence
ar_star = pareto.archive(['min', 'max', 'min'])
for [p, v] in ar:
    ar_star[p[0] - 1.0, p[1] + 1.0, p[2] - 1.0] = v

print('GD:', ar.gd(ar_star))
print('STDGD:', ar.std_gd(ar_star))
print('IGD:', ar.igd(ar_star))
print('STDGD:', ar.std_igd(ar_star))
print('Hausdorff:', ar.hausdorff(ar_star))
print('IGD+:', ar.igd_plus(ar_star))
print('STDIGD+:', ar.std_igd_plus(ar_star))

# Distribution
print("Uniformity:", ar.uniformity())
print("Average distance:", ar.average_distance())
print("Average nearest distance:", ar.average_nearest_distance(5))
near_origin = next(ar.find_nearest(pareto.point([0.0, 0.0])))
print("Crowding distance:", ar.crowding_distance(near_origin[0]))
print("Average crowding distance:", ar.average_crowding_distance())

# Correlation
print('Direct conflict(0,1):', ar.direct_conflict(0, 1))
print('Normalized direct conflict(0,1):', ar.normalized_direct_conflict(0, 1))
print('Maxmin conflict(0,1):', ar.maxmin_conflict(0, 1))
print('Normalized maxmin conflict(0,1):', ar.normalized_maxmin_conflict(0, 1))
print('Non-parametric conflict(0,1):', ar.conflict(0, 1))
print('Normalized conflict(0,1):', ar.normalized_conflict(0, 1))

print('Direct conflict(1,2):', ar.direct_conflict(1, 2))
print('Normalized direct conflict(1,2):', ar.normalized_direct_conflict(1, 2))
print('Maxmin conflict(1,2):', ar.maxmin_conflict(1, 2))
print('Normalized maxmin conflict(1,2):', ar.normalized_maxmin_conflict(1, 2))
print('Non-parametric conflict(1,2):', ar.conflict(1, 2))
print('Normalized conflict(1,2):', ar.normalized_conflict(1, 2))

# Queries
for [k, v] in ar.find_intersection(ar.ideal(), pareto.point([-2.3912, 0.395611, 2.78224])):
    print(k, "->", v)

for [k, v] in ar.find_within(ar.ideal(), pareto.point([-2.3912, 0.395611, 2.78224])):
    print(k, "->", v)

for [k, v] in ar.find_disjoint(ar.worst(), pareto.point([+0.71, +1.19, +0.98])):
    print(k, "->", v)

for [k, v] in ar.find_nearest(pareto.point([-2.3912, 0.395611, 2.78224]), 2):
    print(k, "->", v)

for [k, v] in ar.find_nearest(pareto.point([2.5, 2.5, 2.5])):
    print(k, "->", v)

for [k, v] in ar.find_dominated(pareto.point([-10, +10, -10])):
    print(k, "->", v)

for i in range(ar.dimensions()):
    print('Ideal element in dimension', i, ': ', ar.ideal_element(i)[0])
    print('Nadir element in dimension', i, ': ', ar.nadir_element(i)[0])
    print('Worst element in dimension', i, ': ', ar.worst_element(i)[0])

# Relational operators
ar3 = pareto.archive(ar)
if ar == ar3:
    print('The containers have the same elements')
else:
    if len(ar) != len(ar3):
        print('The containers do not have the same elements')
    else:
        print('The containers might not have the same elements')
        # You need a for loop after here to make sure

ar4 = pareto.archive()
for [k, v] in ar:
    ar4[k] = v

if ar == ar4:
    print('The containers have the same elements')
else:
    if len(ar) != len(ar4):
        print('The containers do not have the same elements')
    else:
        print('The containers might not have the same elements')
        # You need a for loop after here to make sure

if ar_star < ar:
    print('ar* dominates ar')
else:
    print('ar* does not dominate ar')

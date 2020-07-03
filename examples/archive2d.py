import pyfront

ar = pyfront.archive(['minimization', 'maximization'], 100)

ar[0.68322, 0.545438] = 17
ar[-0.204484, 0.819538] = 8
ar[1.10158, 0.977164] = 36
ar[-2.01773, -1.25209] = 27
ar[-1.18976, 1.04908] = 15
ar[1.00638, -0.988565] = 33
ar[-1.20382, 0.288829] = 23
ar[-0.230579, -1.90872] = 16
ar[-1.25501, 0.366831] = 17
ar[-1.5753, -0.600874] = 31
ar[0.623662, -0.835896] = 14
ar[-2.31613, -0.219302] = 8
ar[1.51927, -0.923787] = 12
ar[0.288334, -0.215865] = 7
ar[0.187418, -0.619002] = 40
ar[-0.283076, -1.58758] = 2
ar[-0.477429, 0.670057] = 7
ar[0.516849, 1.18725] = 0
ar[1.42374, 0.183388] = 27

del ar[-2.31613, -0.219302]

print(ar)
print(len(ar), 'elements in the front')
if ar:
    print('Front is not empty')
print(ar.dimensions(), 'dimensions')
print('All' if ar.is_minimization() else 'Not all', 'dimensions are minimization')
print('Dimension 0 is', 'minimization' if ar.is_minimization(0) else 'not minimization')
print('Dimension 1 is', 'maximization' if ar.is_maximization(1) else 'not maximization')

print('Ideal point:', ar.ideal())
print('Ideal point in dimension 0:', ar.dimension_ideal(0))
print('Ideal point in dimension 1:', ar.dimension_ideal(1))
print('Nadir point:', ar.nadir())
print('Nadir point in dimension 0:', ar.dimension_nadir(0))
print('Nadir point in dimension 1:', ar.dimension_nadir(1))
print('Worst point:', ar.nadir())
print('Worst point in dimension 0:', ar.dimension_worst(0))
print('Worst point in dimension 1:', ar.dimension_worst(1))

ar[-2.31613, -0.219302] = 8

for [point, value] in ar:
    print(point, '->', value)

for [point, value] in reversed(ar):
    print(point, '->', value)

for [point, value] in ar.find_intersection(ar.ideal(), ar.point([-1.21188, -1.24192])):
    print(point, '->', value)

for [point, value] in ar.find_within(ar.ideal(), ar.point([-1.21188, -1.24192])):
    print(point, '->', value)

for [point, value] in ar.find_disjoint(ar.worst(), ar.point([+0.71, +1.19])):
    print(point, '->', value)

for [point, value] in ar.find_nearest(ar.point([-1.21188, -1.24192]), 2):
    print(point, '->', value)

p = ar.get_nearest(ar.point([2.5, 2.5]))
print(p[0], '->', p[1])

p1 = ar.point([0, 0])
p2 = ar.point([1, 1])
print('p1 dominates p2' if p1.dominates(p2) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2) else 'p1 does not non-dominate p2')

print('ar dominates p2' if ar.dominates(p2) else 'ar does not dominate p2')
print('ar strongly dominates p2' if ar.strongly_dominates(p2) else 'ar does not strongly dominate p2')
print('ar non-dominates p2' if ar.non_dominates(p2) else 'ar does not non-dominate p2')
print('ar is partially dominated by p2' if ar.is_partially_dominated_by(p2) else 'ar is not is partially dominated by p2')
print('ar is completely dominated by p2' if ar.is_completely_dominated_by(p2) else 'ar is not is completely dominated by p2')

ar2 = pyfront.archive(['minimization', 'maximization'], 100)
for [p, v] in ar:
    ar2[p-1] = v

print('ar dominates ar2' if ar.dominates(ar2) else 'ar does not dominate ar2')
print('ar strongly dominates ar2' if ar.strongly_dominates(ar2) else 'ar does not strongly dominate ar2')
print('ar non-dominates ar2' if ar.non_dominates(ar2) else 'ar does not non-dominate ar2')
print('ar is partially dominated by ar2' if ar.is_partially_dominated_by(ar2) else 'ar is not is partially dominated by ar2')
print('ar is completely dominated by ar2' if ar.is_completely_dominated_by(ar2) else 'ar is not is completely dominated by ar2')

print('Exact hypervolume:', ar.hypervolume(ar.nadir()))
print('Hypervolume approximation (10000 samples):', ar.hypervolume(ar.nadir(), 10000))

print('C-metric:', ar.coverage(ar2))
print('Coverage ratio:', ar.coverage_ratio(ar2))

print(ar)
for [k,v] in ar:
    print(k, v)

ar_star = pyfront.archive(['minimization', 'maximization'], 100)
for [p, v] in ar:
    ar_star[p[0] - 1.0, p[1] + 1.0] = v

print('GD:', ar.gd(ar_star))
print('STDGD:', ar.std_gd(ar_star))
print('IGD:', ar.igd(ar_star))
print('STDGD:', ar.std_igd(ar_star))
print('Hausdorff:', ar.hausdorff(ar_star))
print('IGD+:', ar.igd_plus(ar_star))
print('STDIGD+:', ar.std_igd_plus(ar_star))

print("Uniformity:", ar.uniformity())
print("Average distance:", ar.average_distance())
print("Average nearest distance:", ar.average_nearest_distance(5))
near_origin = ar.get_nearest(ar.point([0.0, 0.0]))
print("Crowding distance:", ar.crowding_distance(near_origin[0]))
print("Average crowding distance:", ar.average_crowding_distance())

print('Direct conflict:', ar.direct_conflict(0, 1))
print('Normalized direct conflict:', ar.normalized_direct_conflict(0,1))
print('Maxmin conflict:', ar.maxmin_conflict(0, 1))
print('Normalized maxmin conflict:', ar.normalized_maxmin_conflict(0,1))
print('Non-parametric conflict:', ar.conflict(0, 1))
print('Normalized conflict:', ar.normalized_conflict(0, 1))
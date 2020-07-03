import pyfront

pf = pyfront.front(['minimization', 'maximization'])

pf[0.68322, 0.545438] = 17
pf[-0.204484, 0.819538] = 8
pf[1.10158, 0.977164] = 36
pf[-2.01773, -1.25209] = 27
pf[-1.18976, 1.04908] = 15
pf[1.00638, -0.988565] = 33
pf[-1.20382, 0.288829] = 23
pf[-0.230579, -1.90872] = 16
pf[-1.25501, 0.366831] = 17
pf[-1.5753, -0.600874] = 31
pf[0.623662, -0.835896] = 14
pf[-2.31613, -0.219302] = 8
pf[1.51927, -0.923787] = 12
pf[0.288334, -0.215865] = 7
pf[0.187418, -0.619002] = 40
pf[-0.283076, -1.58758] = 2
pf[-0.477429, 0.670057] = 7
pf[0.516849, 1.18725] = 0
pf[1.42374, 0.183388] = 27

del pf[-2.31613, -0.219302]

print(pf)
print(len(pf), 'elements in the front')
if pf:
    print('Front is not empty')
print(pf.dimensions(), 'dimensions')
print('All' if pf.is_minimization() else 'Not all', 'dimensions are minimization')
print('Dimension 0 is', 'minimization' if pf.is_minimization(0) else 'not minimization')
print('Dimension 1 is', 'maximization' if pf.is_maximization(1) else 'not maximization')

print('Ideal point:', pf.ideal())
print('Ideal point in dimension 0:', pf.dimension_ideal(0))
print('Ideal point in dimension 1:', pf.dimension_ideal(1))
print('Nadir point:', pf.nadir())
print('Nadir point in dimension 0:', pf.dimension_nadir(0))
print('Nadir point in dimension 1:', pf.dimension_nadir(1))

pf[-2.31613, -0.219302] = 8

for [point, value] in pf:
    print(point, '->', value)

for [point, value] in reversed(pf):
    print(point, '->', value)

for [point, value] in pf.find_intersection(pf.ideal(), pf.point([-1.21188, -1.24192])):
    print(point, '->', value)

for [point, value] in pf.find_within(pf.ideal(), pf.point([-1.21188, -1.24192])):
    print(point, '->', value)

for [point, value] in pf.find_disjoint(pf.worst(), pf.point([+0.71, +1.19])):
    print(point, '->', value)

for [point, value] in pf.find_nearest(pf.point([-1.21188, -1.24192]), 2):
    print(point, '->', value)

p = pf.get_nearest(pf.point([2.5, 2.5]))
print(p[0], '->', p[1])

p1 = pf.point([0, 0])
p2 = pf.point([1, 1])
print('p1 dominates p2' if p1.dominates(p2) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2) else 'p1 does not non-dominate p2')

print('pf dominates p2' if pf.dominates(p2) else 'pf does not dominate p2')
print('pf strongly dominates p2' if pf.strongly_dominates(p2) else 'pf does not strongly dominate p2')
print('pf non-dominates p2' if pf.non_dominates(p2) else 'pf does not non-dominate p2')
print('pf is partially dominated by p2' if pf.is_partially_dominated_by(p2) else 'pf is not is partially dominated by p2')
print('pf is completely dominated by p2' if pf.is_completely_dominated_by(p2) else 'pf is not is completely dominated by p2')

pf2 = pyfront.front(['minimization', 'maximization'])
for [p,v] in pf:
    pf2[p-1] = v

print('pf dominates pf2' if pf.dominates(pf2) else 'pf does not dominate pf2')
print('pf strongly dominates pf2' if pf.strongly_dominates(pf2) else 'pf does not strongly dominate pf2')
print('pf non-dominates pf2' if pf.non_dominates(pf2) else 'pf does not non-dominate pf2')
print('pf is partially dominated by pf2' if pf.is_partially_dominated_by(pf2) else 'pf is not is partially dominated by pf2')
print('pf is completely dominated by pf2' if pf.is_completely_dominated_by(pf2) else 'pf is not is completely dominated by pf2')

print('Exact hypervolume:', pf.hypervolume(pf.nadir()))
print('Hypervolume approximation (10000 samples):', pf.hypervolume(pf.nadir(), 10000))

print('C-metric:', pf.coverage(pf2))
print('Coverage ratio:', pf.coverage_ratio(pf2))

print(pf)
for [k,v] in pf:
    print(k, v)

pf_star = pyfront.front(['minimization', 'maximization'])
for [p, v] in pf:
    pf_star[p[0] - 1.0, p[1] + 1.0] = v

print('GD:', pf.gd(pf_star))
print('STDGD:', pf.std_gd(pf_star))
print('IGD:', pf.igd(pf_star))
print('STDGD:', pf.std_igd(pf_star))
print('Hausdorff:', pf.hausdorff(pf_star))
print('IGD+:', pf.igd_plus(pf_star))
print('STDIGD+:', pf.std_igd_plus(pf_star))

print("Uniformity:", pf.uniformity())
print("Average distance:", pf.average_distance())
print("Average nearest distance:", pf.average_nearest_distance(5))
near_origin = pf.get_nearest(pf.point([0.0,0.0]))
print("Crowding distance:", pf.crowding_distance(near_origin[0]))
print("Average crowding distance:", pf.average_crowding_distance())

print('Direct conflict:', pf.direct_conflict(0,1))
print('Normalized direct conflict:', pf.normalized_direct_conflict(0,1))
print('Maxmin conflict:', pf.maxmin_conflict(0,1))
print('Normalized maxmin conflict:', pf.normalized_maxmin_conflict(0,1))
print('Non-parametric conflict:', pf.conflict(0,1))
print('Normalized conflict:', pf.normalized_conflict(0,1))
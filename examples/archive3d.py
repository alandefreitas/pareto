import pyfront

ar = pyfront.archive(['min', 'max', 'min'], 100)

ar[0.00139845, 0.731828, -0.417717] = 39
ar[0.241096, 0.378685, 0.625521] = 9
ar[-0.844514, 0.93904, -1.32369] = 17
ar[-2.07746, 0.0428126, 0.601997] = 3
ar[-0.23166, 0.246729, 0.0105463] = 21
ar[1.60364, 0.2668, 0.194744] = 40
ar[-0.206962, 1.17536, -0.541873] = 30
ar[0.96463, 0.501958, 0.10121] = 15
ar[1.05309, 0.643423, 1.18504] = 40
ar[-1.45982, -2.16546, -1.03786] = 18
ar[0.494578, -0.417599, -0.975301] = 39
ar[0.182242, -0.339061, -0.453524] = 29
ar[-1.99458, 0.536806, -0.329274] = 2
ar[0.585719, 1.12752, 1.28625] = 11
ar[0.754551, -0.718838, 1.11063] = 15
ar[-0.891867, -0.244561, 0.188812] = 34
ar[-0.749172, -0.0725296, 1.00695] = 30
ar[-0.228034, -0.752103, 0.845463] = 35
ar[-0.0954375, 0.570624, 0.519089] = 24
ar[-0.685613, 0.197224, 0.00797375] = 29
ar[-0.889068, 0.129467, -0.41679] = 27
ar[1.54816, -1.19259, -0.458453] = 26
ar[1.07679, -0.437278, 0.165807] = 12
ar[0.256203, -0.225997, -0.462255] = 33
ar[0.00322445, -0.570125, -0.471465] = 24
ar[0.134622, -0.143287, 1.12839] = 28
ar[-0.478452, 2.0099, -1.19138] = 34
ar[1.33658, -0.949733, 1.52429] = 37
ar[-0.393048, -0.6078, -1.04685] = 21
ar[-0.462544, -0.142344, -0.796835] = 22
ar[1.11088, -0.593916, -0.612791] = 36
ar[0.802046, -1.78496, 0.623616] = 38
ar[-0.560324, -1.13717, 0.968791] = 27
ar[-1.50408, 0.539859, 0.462301] = 21
ar[1.46727, 1.068, 0.311819] = 24
ar[-0.885645, 0.473441, 0.0844403] = 39
ar[-2.27298, 0.613653, -0.83334] = 3
ar[-0.381352, 0.165189, 0.135104] = 40
ar[-0.652758, -0.217548, -1.13146] = 19
ar[1.57291, -0.674738, -1.70623] = 38
ar[0.354773, -0.947818, -0.324024] = 17
ar[0.156779, -0.752439, 0.497127] = 0
ar[-0.571312, 0.965466, -0.87946] = 35
ar[-1.08102, 1.12723, 0.943715] = 8
ar[0.517248, -0.716396, -0.217674] = 13
ar[0.12237, -0.0683769, 1.17239] = 14
ar[-1.18463, 0.0203785, -0.757749] = 17
ar[0.0942627, 2.38411, -0.108849] = 12
ar[0.50611, -0.556946, 0.871486] = 11
ar[0.0279349, -0.174863, -0.242363] = 19

del ar[0.0279349, -0.174863, -0.242363]

print(ar)
print(len(ar), 'elements in the front')
if ar:
    print('Front is not empty')
print(ar.dimensions(), 'dimensions')
print('All' if ar.is_minimization() else 'Not all', 'dimensions are minimization')
print('Dimension 0 is', 'minimization' if ar.is_minimization(0) else 'not minimization')
print('Dimension 1 is', 'maximization' if ar.is_maximization(1) else 'not maximization')
print('Dimension 2 is', 'minimization' if ar.is_minimization(2) else 'not minimization')

print('Ideal point:', ar.ideal())
print('Ideal point in dimension 0:', ar.dimension_ideal(0))
print('Ideal point in dimension 1:', ar.dimension_ideal(1))
print('Ideal point in dimension 2:', ar.dimension_ideal(2))
print('Nadir point:', ar.nadir())
print('Nadir point in dimension 0:', ar.dimension_nadir(0))
print('Nadir point in dimension 1:', ar.dimension_nadir(1))
print('Nadir point in dimension 2:', ar.dimension_nadir(2))
print('Worst point:', ar.nadir())
print('Worst point in dimension 0:', ar.dimension_worst(0))
print('Worst point in dimension 1:', ar.dimension_worst(1))
print('Worst point in dimension 2:', ar.dimension_worst(2))

ar[0.0279349, -0.174863, -0.242363] = 8

for [point, value] in ar:
    print(point, '->', value)

for [point, value] in reversed(ar):
    print(point, '->', value)

for [point, value] in ar.find_intersection(ar.ideal(), ar.point([0.517248, -0.716396, -0.217674])):
    print(point, '->', value)

for [point, value] in ar.find_within(ar.ideal(), ar.point([0.517248, -0.716396, -0.217674])):
    print(point, '->', value)

for [point, value] in ar.find_disjoint(ar.worst(), ar.point([+0.71, +1.19, +0.98])):
    print(point, '->', value)

for [point, value] in ar.find_nearest(ar.point([0.517248, -0.716396, -0.217674]), 2):
    print(point, '->', value)

p = ar.get_nearest(ar.point([2.5, 2.5, 2.5]))
print(p[0], '->', p[1])

p1 = ar.point([0, 0, 0])
p2 = ar.point([1, 1, 1])
print('p1 dominates p2' if p1.dominates(p2) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2) else 'p1 does not non-dominate p2')

print('ar dominates p2' if ar.dominates(p2) else 'ar does not dominate p2')
print('ar strongly dominates p2' if ar.strongly_dominates(p2) else 'ar does not strongly dominate p2')
print('ar non-dominates p2' if ar.non_dominates(p2) else 'ar does not non-dominate p2')
print('ar is partially dominated by p2' if ar.is_partially_dominated_by(p2) else 'ar is not is partially dominated by p2')
print('ar is completely dominated by p2' if ar.is_completely_dominated_by(p2) else 'ar is not is completely dominated by p2')

ar2 = pyfront.archive(['min', 'max', 'min'], 100)
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
for [k, v] in ar:
    print(k, v)

ar_star = pyfront.archive(['min', 'max', 'min'], 100)
for [p, v] in ar:
    ar_star[p[0] - 1.0, p[1] + 1.0, p[2] - 1.0] = v

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
near_origin = ar.get_nearest(ar.point([0.0, 0.0, 0.0]))
print("Crowding distance:", ar.crowding_distance(near_origin[0]))
print("Average crowding distance:", ar.average_crowding_distance())

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
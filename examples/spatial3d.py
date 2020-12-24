import pareto

# Short example
m = pareto.spatial_map()
m[-2.5, -1.5] = 17
m[-2.1, -0.5] = 32
m[-1.6, 0.9] = 36
m[-0.6, 0.9] = 13
m[-0.5, 0.8] = 32
print("Closest elements to [0, 0]:")
for [k, v] in m.find_nearest(pareto.point([0.,0.]), 2):
    print(k, ":", v)

print("Elements between [-1, -1] and [+1, +1]:")
for [k, v] in m.find_intersection(pareto.point([-1.,-1.]), pareto.point([+1, +1])):
    print(k, ":", v)


# Constructor
m = pareto.spatial_map()

# Element access
m[-2.57664, -1.52034, 0.600798] = 17
m[-2.14255, -0.518684, -2.92346] = 32
m[-1.63295, 0.912108, -2.12953] = 36
m[-0.653036, 0.927688, -0.813932] = 13
m[-0.508188, 0.871096, -2.25287] = 32
m[-2.55905, -0.271349, 0.898137] = 6
m[-2.31613, -0.219302, 0] = 8
m[-0.639149, 1.89515, 0.858653] = 10
m[-0.401531, 2.30172, 0.58125] = 39
m[0.0728106, 1.91877, 0.399664] = 25
m[-1.09756, 1.33135, 0.569513] = 20
m[-0.894115, 1.01387, 0.462008] = 11
m[-1.45049, 1.35763, 0.606019] = 17
m[0.152711, 1.99514, -0.112665] = 13
m[-2.3912, 0.395611, 2.78224] = 11
m[-0.00292544, 1.29632, -0.578346] = 20
m[0.157424, 2.30954, -1.23614] = 6
m[0.453686, 1.02632, -2.24833] = 30
m[0.693712, 1.12267, -1.37375] = 12
m[1.49101, 3.24052, 0.724771] = 24

print('Element access:', m[1.49101, 3.24052, 0.724771])

# Iterators
print('Iterators')
for [point, value] in m:
    print(point, '->', value)

print('Reversed Iterators')
for [point, value] in reversed(m):
    print(point, '->', value)

# Capacity and Reference Points
if m:
    print('Map is not empty')
else:
    print('Map is empty')
print(len(m), 'elements in the spatial map')
print(m.dimensions(), 'dimensions')
for i in range(m.dimensions()):
    print('Min value in dimension', i, ': ', m.min_value(i))
    print('Max value in dimension', i, ': ', m.max_value(i))

# Modifiers
m.insert([pareto.point([1.49101, 3.24052, 0.724771]), 24])
del m[1.49101, 3.24052, 0.724771]

# Lookup and queries
for [point, value] in m.find_intersection(pareto.point([-10, -10, -10]), pareto.point([-1.21188, -1.24192, +10])):
    print(point, '->', value)

for [point, value] in m.find_within(pareto.point([-10, -10, -10]), pareto.point([-1.21188, -1.24192, +10])):
    print(point, '->', value)

for [point, value] in m.find_disjoint(pareto.point([+0.2, +0.19, -1]), pareto.point([+0.71, +1.19, +10])):
    print(point, '->', value)

for [point, value] in m.find_nearest(pareto.point([-1.21188, -1.24192, 10]), 2):
    print(point, '->', value)

for [point, value] in m.find_nearest(pareto.point([2.5, 2.5, 10])):
    print(point, '->', value)

# Relational operators
m2 = pareto.spatial_map(m)
if m == m2:
    print('The containers have the same elements')
else:
    if len(m) != len(m2):
        print('The containers do not have the same elements')
    else:
        print('The containers might not have the same elements')
        # You need a for loop after here to make sure

m3 = pareto.spatial_map()
for [k, v] in m:
    m3[k] = v

if m == m3:
    print('The containers have the same elements')
else:
    if len(m) != len(m3):
        print('The containers do not have the same elements')
    else:
        print('The containers might not have the same elements')
        # You need a for loop after here to make sure
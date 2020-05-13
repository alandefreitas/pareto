import pyfront
from pyfront import randn, randi

# Creating pareto front
pf = pyfront.front2d([[[0.2, 0.5], 7], [[-0.5, 2.5], 8]])  # or: pyfront.front(2)

# Insert some more points
for i in range(0, 40):
    pf.insert([randn(), randn()], randi())

# Show pareto front
print(pf)
for [point, value] in pf:
    print('point:', point, ': value:', value, end=', ')
print('\n')

# Get some properties
print('size:', pf.size())
print('hypervolume:', pf.hypervolume())
print('ideal:', pf.ideal())
print('nadir:', pf.nadir())
print('worst:', pf.worst())
print('')

# Check if the set dominates a point
print('pf.dominates([5.5, 5.5]):', pf.dominates([5.5, 5.5]))
print('pf.dominates([-5.5, -5.5]):', pf.dominates([-5.5, -5.5]))
print('')

# Show points in the box {0,0} to {5,5}
for [point, value] in pf.find_intersection([0, 0], [5, 5]):
    print(point,":",value, end=', ')
print('\n')

# Show point nearest to {2.5,2.5}
for point_and_value in pf.find_nearest([2.5, 2.5]):
    print(point_and_value)
print('')

# Show 5 points nearest to {2.5,2.5}
print(pf.get_nearest([2.5, 2.5], 5))
print('')

# Remove point closest to 0.0
print(pf.get_nearest([0.0,0.0]))
pf.erase(pf.find_nearest([0.0,0.0]))

# Remove the next closest
it = pf.find_nearest([0.0,0.0])
pf.erase(it)

# Show final closest
print(pf.get_nearest([0.0,0.0]))
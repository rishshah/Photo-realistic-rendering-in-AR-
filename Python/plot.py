from mpl_toolkits.mplot3d import axes3d
import matplotlib.pyplot as plt
from numpy import genfromtxt

fig = plt.figure()

ax = fig.add_subplot(111, projection ='3d')
my_data = genfromtxt('abc.txt', delimiter=' ')
X = my_data[:,0]
Y = my_data[:,1]
Z = my_data[:,2]
ax.scatter(X,Y,Z,c='r', marker='o', s=0.5)
plt.show()
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from numpy import genfromtxt
import numpy as np
import random
import math

# Global constants
INF = 10000000000000

# INPUT

my_data = genfromtxt('abc.txt', delimiter=' ')
X = my_data[:,0] * 100
Y = my_data[:,1] * 100
Z = my_data[:,2] * 100

num_total_iterations = 50
num_points = len(X)
tolerance_limit = 10
good_num_points = 20 


# Indirect functions(Private)
def dist2plane(point, plane):
    num = abs(plane[0]*point[0] + plane[1]*point[1] + plane[2] - point[2])
    den = math.sqrt(plane[0]*plane[0] + plane[1]*plane[1] + 1)
    return num/den


# Direct funtions (Public)
def get3Points():
    return random.sample(list(np.arange(num_points)),  3)
    
def fitPlane(indices):
    tmp_A = []
    tmp_b = []
    for i in indices:
        tmp_A.append([X[i], Y[i], 1])
        tmp_b.append(Z[i])
    
    b = np.matrix(tmp_b).T
    A = np.matrix(tmp_A)
    fit = (A.T * A).I * A.T * b
    errors = b - A * fit
    residual = np.linalg.norm(errors)

    print ("solution: {0} x + {1} y + {2} = z".format(fit[0], fit[1], fit[2]))
    print ("errors: {0}".format(sum(abs(errors))))
    print ("residual:  {0}".format(residual))
    return [fit[0], fit[1], fit[2], sum(abs(errors)), residual]

def good_consensus(l):
    return len(l) > good_num_points

def fits_model(i, plane):
    p = np.array([X[i], Y[i], Z[i]])
    return dist2plane(p, plane[0:3]) < tolerance_limit


# Temporary And Output Variables
iterations = 0
best_error = INF
best_model = None
best_consensus_set = None


# Algortithm RANSAC for one plane
while iterations < num_total_iterations:

    maybe_inliers = get3Points()
    maybe_model = fitPlane(maybe_inliers)
    consensus_set = maybe_inliers

    for i in range(0,num_points):
        if i not in maybe_inliers and fits_model(i, maybe_model): 
            consensus_set.append(i)

    if good_consensus(consensus_set):
        this_model = fitPlane(consensus_set)
        this_error = this_model[-2]
        if this_error < best_error:
            best_model = this_model
            best_consensus_set = consensus_set
            best_error = this_error

    iterations = iterations + 1

print("BEST ERROR: {0}".format(best_error))
print("BEST CONSENSUS: {0}".format(len(best_consensus_set)))


# Plot points
X = [X[i] for i in best_consensus_set]
Y = [Y[i] for i in best_consensus_set]
Z = [Z[i] for i in best_consensus_set]

plt.figure()
ax = plt.subplot(111, projection='3d')
ax.scatter(X, Y, Z, color='b')

# Plot plane
fit = best_model

xlim = ax.get_xlim()
ylim = ax.get_ylim()
X,Y = np.meshgrid(np.arange(xlim[0], xlim[1]),
                  np.arange(ylim[0], ylim[1]))
Z = np.zeros(X.shape)
for r in range(X.shape[0]):
    for c in range(X.shape[1]):
        Z[r,c] = fit[0] * X[r,c] + fit[1] * Y[r,c] + fit[2]
ax.plot_wireframe(X,Y,Z, color='k')

ax.set_xlabel('x')
ax.set_ylabel('y')
ax.set_zlabel('z')
plt.show()
import numpy as np
import time

A = np.random.rand(10, 20)
B = np.random.rand(20, 30)

start = time.time()
C = A @ B
end = time.time()

print("Time: ", end - start)
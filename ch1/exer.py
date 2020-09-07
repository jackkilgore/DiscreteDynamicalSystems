from math import *

PARAM = 3.3333333333

def h(x):
  return PARAM*x*(1-x)

def iter_h(init_x,n):
  result = init_x
  for i in range(n):
    result = h(result)
  return result


# Tests

result = iter_h(0.1,503)
print(result)

from py_ecc.bn128 import *

x = multiply(G1, 2)
y = multiply(G2, 1)

a = multiply(G1, 1)
b = multiply(G2, 2)

print(pairing(y, x))
print(pairing(b, a))

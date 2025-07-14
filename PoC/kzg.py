from functools import reduce
from math import prod
from py_ecc.bn128 import *
import random

def GQ_polyval(p, x):
  return sum([ (p_i * x ** i) % curve_order for i, p_i in enumerate(p) ]) % curve_order

def gcd(a, b):
  if b == 0:
    return a
  x, y = b, a % b
  return gcd(x, y)

def egcd(a, b):
  if a == 0:
    return b, 0, 1
  
  r, x1, y1 = egcd(b % a, a)
  x = y1 - x1 * (b//a)
  y = x1
  
  return r, x, y

def mod_reduce(p):
  p = p.copy()
  p = list(map(lambda x : int(x) % curve_order, p))
  while len(p) > 1 and p[-1] == 0: p.pop()
  return p

def mod_inv(x):
  return egcd(x, curve_order)[1] % curve_order

def mod_div(a, b):
  return (a * mod_inv(b)) % curve_order

def mod_polydiv(n, d):
  q = [0]
  r = n
  
  while r[-1] != 0 and len(r) >= len(d):
    t = ([0] * (len(r) - len(d))).copy() + [mod_div(r[-1] % curve_order, d[-1] % curve_order)]
    q = mod_polyadd(q, t)
    r = mod_polysub(r, mod_polymul(d, t))
  
  return q,r

def mod_polyadd(a, b):
  if len(b) > len(a):
    a, b = b, a
  c = a.copy()
  for term, coeff in enumerate(b):
    c[term] = (c[term] + coeff) % curve_order
  return mod_reduce(c)

def mod_polysub(a, b):
  return mod_polyadd(a, mod_polymul(b, [-1]))

def mod_polymul(a, b):
  if len(b) > len(a):
    a, b = b, a
  
  c = [0]
  for term, coeff in enumerate(a):
    t = [ (i * coeff) % curve_order for i in b ]
    u = ([0] * term).copy() + t
    c = mod_polyadd(u, c)
  
  return c

def polyfit(A):
  poly = [0]
  for i in range(len(A)):
    x_i, y_i = A[i]
    numerator = [y_i]
    denominator = 1
    
    for j in range(len(A)):
      if i == j: continue
      x_j, y_j = A[j]
      numerator = mod_polymul([-x_j, 1], numerator)
      denominator = (denominator * (x_i - x_j)) % curve_order
    
    q, r = mod_polydiv(numerator, [denominator])
    assert r == [0]
    poly = mod_polyadd(poly, q)
  
  return poly

def trusted_setup(s, N):
  # This part can probably be optimized by using previous results
  x = [ multiply(G1, pow(s, i, curve_order)) for i in range(0, N) ]
  y = [ multiply(G2, pow(s, i, curve_order)) for i in range(0, N) ]
  return x,y

def EC_polyval(p, x):
  return reduce(add, [ multiply(x_i, p_i) if p_i >= 0 else neg(multiply(x_i, -p_i)) for p_i, x_i in zip(p, x) ])

def GQ_polyval(p, x):
  return sum([ (p_i * (x ** i)) % curve_order for i, p_i in enumerate(p) ]) % curve_order

def commit(p, x):
  return EC_polyval(p, x)

def create_proof(p, x, A):
  # compute a general q(x) by operating on the coefficients
  # then use the trusted setup to actually evaluate it at q(s)
  I = polyfit(A)
  Z = reduce(mod_polymul, [[-p[0], 1] for p in A])
  q,r = mod_polydiv(mod_polysub(p, I), Z)
  assert r == [0]
  return EC_polyval(q, x)

def verify(p_s, q_s, x, y, A):
  I = polyfit(A)
  Z = reduce(mod_polymul, [[-p[0], 1] for p in A])
  v_1 = pairing(EC_polyval(Z, y), q_s)
  v_2 = pairing(G2, add(p_s, neg(EC_polyval(I, x))))
  return v_1 == v_2

def encode_into_poly(text, offset=0):
  return list([ (i + offset, c) for i, c in enumerate(map(ord, text)) ])

print("Creating trusted setup.")
x,y = trusted_setup(random.randint(0, 1024), 64)

msg = "hi there my name is bob, i write programs"
print(f"Committing the message '{msg}'.")
p = polyfit(encode_into_poly(msg))
p_s = commit(p, x)

proof_greeting = create_proof(p, x, encode_into_poly("hi there"))
print("Generated proof of greeting: text contains 'hi there'.")

proof_offers_name = create_proof(p, x, encode_into_poly("my name is", offset=len("hi there ")))
print("Generated proof of name offering: text followed by 'my name is'.")

proof_name = create_proof(p, x, encode_into_poly("bob", offset=len("hi there my name is ")))
print("Generated proof of name: name is specified, 'bob'.")

print("----------------------------")
print("Checking proof against commitment, trusted setups and expected value.")
print("Greeting:", verify(p_s, proof_greeting, x, y, encode_into_poly("hi there")))
print("Offers Name:", verify(p_s, proof_offers_name, x, y, encode_into_poly("my name is", offset=len("hi there "))))
print("Name is Bob:", verify(p_s, proof_name, x, y, encode_into_poly("bob", offset=len("hi there my name is "))))
print("Name is Alice:", verify(p_s, proof_name, x, y, encode_into_poly("alice", offset=len("hi there my name is "))))

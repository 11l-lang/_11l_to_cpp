#
# Shared code for solutions to Project Euler problems
# Copyright (c) Project Nayuki. All rights reserved.
#
# https://www.nayuki.io/page/project-euler-solutions
# https://github.com/nayuki/Project-Euler-solutions
#

import math
BigInt = int

# Returns the number of 1's in the binary representation of
# the non-negative integer x. Also known as Hamming weight.
def popcount(x: int) -> int:
	return bin(x).count("1")

# Given integer x, this returns the integer floor(sqrt(x)).
def sqrt(x: BigInt) -> BigInt:
	assert x >= 0
	i: BigInt = 1
	while i * i <= x:
		i *= 2
	y: BigInt = 0
	while i > 0:
		if (y + i)**2 <= x:
			y += i
		i //= 2
	return y

# Tests whether x is a perfect square, for any integer x.
def is_square(x):
	if x < 0:
		return False
	y = int(math.sqrt(x))
	return y * y == x

# Tests whether the given integer is a prime number.
def is_prime(x):
	if x <= 1:
		return False
	elif x <= 3:
		return True
	elif x % 2 == 0:
		return False
	else:
		for i in range(3, int(math.sqrt(x)) + 1, 2):
			if x % i == 0:
				return False
		return True

# Returns a list of True and False indicating whether each number is prime.
# For 0 <= i <= n, result[i] is True if i is a prime number, False otherwise.
def list_primality(n):
	# Sieve of Eratosthenes
	result = [True] * (n + 1)
	result[0] = result[1] = False
	for i in range(int(math.sqrt(n)) + 1):
		if result[i]:
			for j in range(i * i, len(result), i):
				result[j] = False
	return result

# Returns all the prime numbers less than or equal to n, in ascending order.
# For example: listPrimes(97) = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, ..., 83, 89, 97].
def list_primes(n):
	return [i for i, isprime in enumerate(list_primality(n)) if isprime]

def list_totients(n):
	result = list(range(n + 1))
	for i in range(2, len(result)):
		if result[i] == i:  # i is prime
			for j in range(i, len(result), i):
				result[j] -= result[j] // i
	return result

def binomial(n : BigInt, k : BigInt):
	assert BigInt(0) <= k <= n
	return math.factorial(n) // (math.factorial(k) * math.factorial(n - k))

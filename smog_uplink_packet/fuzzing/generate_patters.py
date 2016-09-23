import itertools

# 0-bit error position  -    1 - 0.0005
patterns = [0]

# 1-bit error position  -   24 - 0.012
for bit_pattern in itertools.combinations(range(24), 1):
	patterns.append(sum(map(lambda x: 1<<x, bit_pattern)))

# 2-bit error positions -  276 - 0.136
for bit_pattern in itertools.combinations(range(24), 2):
	patterns.append(sum(map(lambda x: 1<<x, bit_pattern)))

# 3-bit error positions - 2024 - 1.0
for bit_pattern in itertools.combinations(range(24), 3):
	patterns.append(sum(map(lambda x: 1<<x, bit_pattern)))

# 4-bit error positions - 
for bit_pattern in itertools.combinations(range(24), 4):
	patterns.append(sum(map(lambda x: 1<<x, bit_pattern)))


for i, pattern in enumerate(patterns):
	print("0x%06x, " % pattern, end='')
	if i % 8 == 7:
		print()


# Mapping 2^16 into these bit errors but with an adjustable probability
# Try to be as dense mapping as it could be, so use LSB bit to
# identify n-bit pattern, and the rest of the bit with modulo
# to identify the exact pattern among the n-bit patterns

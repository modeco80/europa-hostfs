def iclass(c):
	return int(c << 26)

def wordToBytes(word: int) -> bytes:
	return bytes(reversed(word.to_bytes(4)))

__all__ = [ 'Mips' ]


# MIPS instruction encoding helpers.
class Mips:
	a0 = 4
	v0 = 2

	# encodes a `nop`. Trivial.
	@staticmethod
	def nop():
		return b'\x00\x00\x00\x00'

	# encodes a `j xxx`
	@staticmethod
	def j(target):
		return wordToBytes(int(iclass(2) | ((int(target) >> 2) & 0x03ffffff)))

	# encodes a `jal xxx`
	@staticmethod
	def jal(target):
		return wordToBytes(int(iclass(3) | ((int(target) >> 2) & 0x03ffffff)))

	@staticmethod
	def move(regDest: int, regSource: int):
		return wordToBytes(0x00000021 | (regDest << 11) | (regSource << 21))

# Upon importing, do a quick set of test assertions to make sure
# that the code here is okay, and works properly.
try:
	assert(Mips.nop() == b'\x00\x00\x00\x00')
	assert(Mips.j(0xff59c) == bytes(reversed(b'\x08\x03\xfd\x67')))
	assert(Mips.jal(0xff59c) == bytes(reversed(b'\x0c\x03\xfd\x67')))
except AssertionError:
	raise RuntimeError('Mips class is broken, fix it.')

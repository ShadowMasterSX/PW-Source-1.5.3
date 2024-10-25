
print 'import octets'

def roundsize(size):
	capacity = 2
	while True:
		size = size >> 1
		if not (size>0): break
		capacity = capacity << 1
	return capacity

class Octets:
	DEFAULT_SIZE = 128

	def reserve(self, size):
		if len(self.buffer) == 0:
			self.buffer.extend(' '*roundsize(size))
		elif size > len(self.buffer):
			self.buffer.extend(' '*(roundsize(size)-len(self.buffer)))

	def replace2(self, data, pos, size):	
		assert self.buffer is not data, 'octets replace itself'
		self.buffer = [' ']*roundsize(size)
		self.count = size
		self.buffer[0:size] = data[pos:pos+size]
		return self

	def replace(self, data):
		return self.replace2(data, 0, len(data))

	def __init__(self, size=DEFAULT_SIZE):
		self.count = 0
		self.buffer = []
		self.reserve(size)
	
	def resize(self, size):
		self.reserve(size)
		self.count = size
		return self

	def size(self):
		return self.count

	def __len__(self):
		return self.count

	def __getitem__(self, i):
		assert i>=0 and i<self.count or 'octets array index overflow'
		return self.buffer[i]

	def __setitem__(self, i, v):
		assert i>=0 and i<self.count or 'octets array index overflow'
		self.buffer[i] = v

	def equals(self, orh):
		if self is orh: return True
		if self.count != orh.count: return False
		if not self.buffer or not orh.buffer: return False
		for i in range(self.count):
			if self.buffer[i] != orh.buffer[i]: return False
		return True

	def capacity(self):
		return len(self.buffer)

	def clear(self):
		self.count = 0
		return self

	def swap(self, octs):
		tmp = self.count
		self.count = octs.count
		octs.count = tmp
		tmp = self.buffer
		self.buffer = octs.buffer
		octs.buffer = tmp
		return self

	def push_back(self, data):
		tmp = self.count + len(data)
		self.reserve(tmp)
		self.buffer[self.count:tmp] = data[:]
		self.count = tmp
		return self

	def erase(self, fromb, tob):
		if tob > fromb and fromb >=0 and tob <= self.count:
			for i in range(fromb, fromb + self.count - tob):
				self.buffer[i] = self.buffer[i + tob - fromb]
			self.count = self.count - tob + fromb
		return self

	def insert2(self, fromb, data, pos, size):
		self.reserve(self.count + size)
		for i in range(self.count - fromb):
			self.buffer[self.count+size-1-i] = self.buffer[self.count-1-i]
		self.buffer[fromb:fromb+size] = data[pos:pos+size]
		self.count = self.count + size
		return self

	def append2(self, data, pos, size):
		return self.insert2(self.count, data, pos, size)

	def insert(self, fromb, data):
		return self.insert2(fromb, data, 0, len(data))

	def append(self, data):
		return self.insert(self.count, data)

	def __deepcopy__(self, memo):
		return self.__class__().replace2(self.buffer, 0, self.count)

	def getbytes2(self, fromb, tob):
		return self.buffer[fromb:tob]

	def getbytes(self):
		return self.getbytes2(0, self.count)

	def getstr(self, fromb, tob):
		r = ''
		for i in range(fromb, tob):
			r = r + self.buffer[i]
		return r

	def __str__(self):
		return self.getstr(0, self.count)

	def hexstr(self):
		tstr = ''
		for i in range(self.count):
			tstr = tstr + '[' + hex(ord(self.buffer[i]))+ ']'
		return tstr

	def hexstr2(self):
		tstr = '{'
		for i in range(self.count):
			tstr = tstr + hex(ord(self.buffer[i]))+ ','
		tstr = tstr + '}'
		return tstr

	def getarray(self):
		return self.buffer

	def getbyte(self, pos):
		return self.buffer[pos]

	def setbyte(self, pos, b):
		self.buffer[pos] = b
		return self

	def marshal(self, os):
		return os.marshalos(self)

	def unmarshal(self, os):
		return os.unmarshalos(self)

def create(data):
	return Octets().replace(data)

"""
x = Octets().replace('ddd')
print x
x.replace('abc')
print x
x.replace('dafsdfasdf')
print x
x.insert(x.size(), 'abc')
print x
x.insert(x.size(), 'def')
print 'size ', x.size(), ' ', x
y = Octets().replace('ABC')
print y
x.insert(x.size(), y.getarray())
print x
import copy
z = copy.deepcopy(x)
print z	
"""


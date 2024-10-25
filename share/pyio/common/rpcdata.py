from mymarshal import Marshal
from copy import deepcopy

print 'import rpcdata'

class Data(Marshal):
	def __init__(self):
		pass
	def __deepcopy__(self, memo):	
		return self.__class__()

class Char(Marshal, int):
	def marshal(self, os):
		os.marshal_int8(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_int8()
	#	return os

class UChar(Marshal, int):
	def marshal(self, os):
		os.marshal_uint8(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_uint8()
	#	return os

class Short(Marshal, int):
	def marshal(self, os):
		os.marshal_int16(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_int16()
	#	return os

class UShort(Marshal, int):
	def marshal(self, os):
		os.marshal_uint16(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_uint16()
	#	return os

class Int(Marshal, int):
	def marshal(self, os):
		os.marshal_int32(self)
		return os
	#def unmarshal(self, os):
	#	print 'int.unmarshal pos is:', os.pos
	#	self = Int(os.unmarshal_int32())
	#	print 'after int unmarshal, self is:', self
	#	return os

class UInt(Marshal, long):
	def marshal(self, os):
		os.marshal_uint32(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_uint32()
	#	return os

class Long(Marshal, long):
	def marshal(self, os):
		os.marshal_int64(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_int64()
	#	return os

class ULong(Marshal, long):
	def marshal(self, os):
		os.marshal_uint64(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_uint64()
	#	return os

class Float(Marshal, float):
	def marshal(self, os):
		os.marshal_float(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_float()
	#	return os

class Double(Marshal, float):
	def marshal(self, os):
		os.marshal_double(self)
		return os
	#def unmarshal(self, os):
	#	self = os.unmarshal_double()
	#	return os

class List(list, Marshal):
	# ltype
	def __init__(self, _type):
		list.__init__(self)
		self._type = _type
	def __deepcopy__(self, memo):
		l = List(self._type)
		#l = self.__class__()
		#l._type = self._type
		for x in self: l.append(deepcopy(x))
		return l
	def marshal(self, os):
		os.compact_uint32(self.__len__())
		for i in range(self.__len__()):
			self.__getitem__(i).marshal(os)
		return os
	def unmarshal(self, os):
		import stub
		inst = eval('stub.' + self._type + '()')
		size = os.uncompact_uint32()
		for i in range(size):
			d = deepcopy(inst)
			d.unmarshal(os)
			self.append(d)
		return os


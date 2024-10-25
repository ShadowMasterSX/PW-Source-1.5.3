print 'import state' 

class State:
	def __init__(self, t):
		self.timeout = t
		self.set = {}
	def __copy__(self):	
		s = State(self.timeout)
		for k,v in self.set.items():
			s.set[k] = v
		return s
	def addprotocoltype(self, _type):
		self.set[_type] = 1
	def typepolicy(self, _type):
		r = self.set.has_key(str(_type))
		return r
	def timepolicy(self, t):
		return self.timeout<0 or t < self.timeout
	def __repr__(self):
		return 'state timeout: ' + str( self.timeout ) + str ( self.set )

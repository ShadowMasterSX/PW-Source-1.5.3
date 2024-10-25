
print 'import mymarshal'

class MarshalException: pass

class Marshal:
	def marshal(osstream):
		assert 0, 'abstract method Marshal::marshal'
	def unmarshal(osstream):
		assert 0, 'abstract method Marshal::unmarshal'


import copy
import md5, hmac
from mycompress import Compress, Decompress

print 'import security'

class Security:
	def __init__(self):
		return

	def __deepcopy__(self, memo):
		return self.__class__()
	
	def setparamter(self, p):
		return

	def getparameter(self):
		return ''

	def update(self, data):
		return data
	
	def final(self, data):
		return data


class NullSecurity(Security):
	def __init__(self):
		Security.__init__(self)
	def __deepcopy__(self, memo):
		return Security.__deepcopy__(self, memo)

class MD5Hash(Security):
	def __init__(self):
		Security.__init__(self)
		self._md5 = md5.new()
	def __deepcopy__(self, memo):
		d = Security.__deepcopy__(self, memo)
		d._md5 = copy.deepcopy(self._md5)
	def update(self, o):
		self._md5.update(str(o))
		return o
	def final(self, o):
		return o.replace(_md5.digest())
	
class HMAC_MD5Hash(Security):
	def __init__(self):
		Security.__init__(self)
		self._hmac = 0
	def __deepcopy__(self, memo):
		d = Security.__deepcopy__(self, memo)
		d._hmac = copy.deepcopy(self._hmac)
	def setparameter(self, o):
		self._hmac = hmac.new(str(o))
		return o
	def update(self, o):
		self._hmac.update(str(o))
		return o
	def final(self, o):
		return o.replace(_hmac.digest())
	

class RandSecurity(Security):
	def __init__(self):
		Security.__init__(self)
	def __deepcopy__(self, memo):
		return Security.__deepcopy__(self, memo)
	def setparamter(self, o):
		return
	def update(self, o):
		return o
class ARCFourSecurity(Security):
	def __init__(self):
		Security.__init__(self)
		self.index1 = 0x00
		self.index2 = 0x00
		self.perm = ['\x00']*256
	def __deepcopy__(self, memo):
		d =  Security.__deepcopy__(self, memo)
		d.index1 = self.index1
		d.index2 = self.index2
		d.perm[:] = self.perm[:]
		return d
	def setparameter(self, o):
		#print 'ARCFOUR setparameter:', o.hexstr()
		keylen = o.size()
		j = 0x00
		for i in range(256):
			self.perm[i] = chr(i)
		for i in range(256):
			j = 0xff&(j + (0xff&(ord(o.buffer[i%keylen]) + ord(self.perm[i]))))
			t = self.perm[i]
			self.perm[i] = self.perm[j]
			self.perm[j] = t
		self.index1 = 0x00
		self.index2 = 0x00
	def update(self, o):
		#print 'ARCFOUR update', o.hexstr()
		keylen = o.size()
		for i in range(keylen):
			self.index1 = ( self.index1 + 1 ) & 0xff
			j1 = ord(self.perm[self.index1]) & 0xff
			self.index2 = ( self.index2 + j1 ) & 0xff
			j2 = ord(self.perm[self.index2]) & 0xff
			#print j1, j2,self.index1, self.index2
			self.perm[self.index2] = chr(j1)
			self.perm[self.index1] = chr(j2)
			o.buffer[i] = chr(ord(o.buffer[i]) ^ ord(self.perm[(j1+j2)&0xff]))
		#print 'after ARCFOUR update', o.hexstr()
		return o

class CompressARCFourSecurity(Security):
	def __init__(self):
		Security.__init__(self)
		self.arc4 = ARCFourSecurity()
		self.comp = Compress()
	def __deepcopy__(self, memo):
		d = Security.__deepcopy__(self, memo)
		d.arc4 = copy.deepcopy(self.arc4)
		d.comp = copy.deepcopy(self.comp)
		return d
	def setparameter(self, p):
		#print 'CompresssARCFourSecurity set p is:', p.hexstr()
		self.arc4.setparameter(p)
	def update(self, o):
		#print 'CompresssARCFourSecurity update o:', o.hexstr()
		#tstr = ''
		#for i in range(o.size()): tstr = tstr + '[' + str(ord(o[i])) + '] '
		#print tstr
		return self.arc4.update(self.comp.final(o))
		#tstr = ''
		#for i in range(o.size()): tstr = tstr + '[' + str(ord(o[i])) + '] '
		#print tstr
		#return o

class DecompressARCFourSecurity(Security):
	def __init__(self):
		Security.__init__(self)
		self.arc4 = ARCFourSecurity()
		self.decomp = Decompress()
	def __deepcopy__(self, memo):
		d = Security.__deepcopy__(self, memo)
		d.arc4 = copy.deepcopy(self.arc4)
		d.decomp = copy.deepcopy(self.decomp)
		return d
	def setparameter(self, p):
		#print 'DecompresssARCFourSecurity set p is:', p.hexstr()
		self.arc4.setparameter(p)
	def update(self, o):
		#print 'DecompresssARCFourSecurity update:', o.hexstr()
		return self.decomp.update(self.arc4.update(o))

#COMPRESSARCFOURSECURITY = 5, DECOMPRESSARCFOURSECURITY = 6,
def create(name):
	if name == '5' or name.upper() == 'COMPRESSARCFOURSECURITY':
		return CompressARCFourSecurity()
	if name == '6' or name.upper() == 'DECOMPRESSARCFOURSECURITY':
		return DecompressARCFourSecurity()
	if name == '2' or name.upper() == 'ARCFOURSECURITY':
		return ARCFourSecurity()
	if name == '0' or name.upper() == 'RANDOMSECURITY':
		return RandSecurity()
	if name == '1' or name.upper() == 'NULLSECURITY':
		return NullSecurity()
	if name == '3' or name.upper() == 'MD5HASH':
		return MD5Hash()
	if name == '4' or name.upper() == 'HMAC_MD5HASH':
		return HMAC_MD5Hash()
	assert 0, 'create security error' + name

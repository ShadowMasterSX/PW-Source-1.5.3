import copy
from mycompress import Decompress, Compress
from security import CompressARCFourSecurity, DecompressARCFourSecurity, ARCFourSecurity
from octets import Octets
import random

def rand_str(n):
	tstr = ''
	for i in range(n):
		tstr = tstr + chr(random.randint(0, 255))
	return tstr


for i in range(1):
	decomp2 = DecompressARCFourSecurity()
	comp2 = CompressARCFourSecurity()
	key = Octets(0).append(rand_str(16))
	comp2.setparameter(key)
	decomp2.setparameter(key)
	os = Octets().append(rand_str(1500))
	osr = Octets().replace(os)
	for j in range(30):
		os = decomp2.update(comp2.update(os))
	print os.equals(osr)

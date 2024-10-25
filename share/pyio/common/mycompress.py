import struct, copy
from octets import Octets

print 'import mycompress'

CTRL_OFF_EOB = 0
MPPC_HIST_LEN = 8192

class Compress:
	def __init__(self):
		self.history = ['\x00'] * MPPC_HIST_LEN
		self.hash = [0] * 256
		self.histptr = 0
		self.legacy_in = 0
	def dump(self):
		print 'legacy_in is ', self.legacy_in
		print 'histptr is ', self.histptr
		print 'hash:'
		for i in range(16):
			tstr = '\t'
			for j in range(16):
				tstr = tstr + '[' + str(self.hash[i*16+j]) + '] '
			print tstr
		print '\n'
	def __deepcopy__(self, memo):
		d = self.__class__()
		d.history = self.history[:]
		d.histptr = self.histptr
		d.legacy_in = self.legacy_in
		d.hash = self.hash[:]
		return d
	def putbits(self, buf, pos, val, n, l):
		#print '\t\tputbits val is', val, 'n is', n, 'l is', l[0], ',llin is', self.legacy_in,',pos is:',pos[0]
		l[0] = l[0] + n
		tstr = struct.pack('!I', (0xffffffffL&((val|0000000000000000L) << (32 -l[0]))))[:]
		c = chr(ord(buf[pos[0]]) | ord(tstr[0]))
		buf[pos[0]:pos[0]+4] = tstr
		buf[pos[0]] = c
		pos[0] = pos[0] + ( l[0] >> 3 )
		l[0] = l[0] & 7
		#ttstr = ''
		#for i in range(self.legacy_in):
		#	ttstr = ttstr + ' ' + str(ord(buf[i]))
		#print '\t\t:', ttstr
	def putlit(self, buf, pos, c, l):
		#print 'putlit c is', c, 'l is', l[0], ',llin is', self.legacy_in
		if c < 0x80:
			self.putbits(buf, pos, c, 8, l)
		else:
			self.putbits(buf, pos, c&0x7f|0x100, 9, l)
	def putoff(self, buf, pos, off, l):
		#print '\tputoff off is', off, 'l is', l[0], ',llin is', self.legacy_in
		if off < 64:
			self.putbits(buf, pos, 0x3c0|off, 10, l)
		elif off < 320:
			self.putbits(buf, pos, 0xe00|(off-64), 12, l)
		else:
			self.putbits(buf, pos, 0xc000|(off-320), 16, l)
	def compress_block(self, buf, pos, isize):
		r = self.histptr + isize
		s = self.histptr
		l = [0]
		buf[pos[0]] = '\x00'
		while r - s > 2:
			q = ord(self.history[s])
			p = self.hash[q]
			#print '===========while=========r is', r, ',s is', s
			self.hash[q] = s
			if p >= s:
				#print '-----case 1-----'
				self.putlit(buf, pos, ord(self.history[self.histptr]), l)
				self.histptr = self.histptr + 1
				s = self.histptr
			else:
				bflag = ( self.history[p] != self.history[s] or self.history[p+1] != self.history[s+1] )
				s = s + 1
				if bflag:
					#print '-----case 2-----'
					self.putlit(buf, pos, ord(self.history[self.histptr]), l)
					self.histptr = self.histptr + 1
				else:
					p = p + 2
					s = s + 1
					if self.history[p] != self.history[s]:
						#print '-----case 3-----'
						self.putlit(buf, pos, ord(self.history[self.histptr]), l)
						self.histptr = self.histptr + 1
						s = self.histptr
					else:
						#print '-----case 4-----'
						p = p + 1
						s = s + 1
						while s < r and self.history[p] == self.history[s]:
							p = p + 1
							s = s + 1
						len = s - self.histptr
						self.histptr = s
						self.putoff(buf, pos, s - p, l)
						if len < 4:
							self.putbits(buf, pos, 0, 1, l)
						elif len < 8:
							self.putbits(buf, pos, 0x08|(len&0x03), 4, l)
						elif len < 16:
							self.putbits(buf, pos, 0x30|(len&0x07), 6, l)
						elif len < 32:
							self.putbits(buf, pos, 0xe0|(len&0x0f), 8, l)
						elif len < 64:
							self.putbits(buf, pos, 0x3c0|(len&0x1f), 10, l)
						elif len < 128:
							self.putbits(buf, pos, 0xf80|(len&0x3f), 12, l)
						elif len < 256:
							self.putbits(buf, pos, 0x3f00|(len&0x7f), 14, l)
						elif len < 512:
							self.putbits(buf, pos, 0xfe00|(len&0xff), 16, l)
						elif len < 1024:
							self.putbits(buf, pos, 0x3fc00|(len&0x1ff), 18, l)
						elif len < 2048:
							self.putbits(buf, pos, 0xff800|(len&0x3ff), 20, l)
						elif len < 4096:
							self.putbits(buf, pos, 0x3ff000|(len&0x7ff), 22, l)
						elif len < MPPC_HIST_LEN:
							self.putbits(buf, pos, 0xffe000|(len&0xfff), 24, l)

		#for ii in range(pos[0]):
		#	print ord(buf[ii])
		if r - s == 2:
			self.putlit(buf, pos, ord(self.history[self.histptr]), l)
			self.histptr = self.histptr + 1
			self.putlit(buf, pos, ord(self.history[self.histptr]), l)
			self.histptr = self.histptr + 1
		elif r - s == 1:
			self.putlit(buf, pos, ord(self.history[self.histptr]), l)
			self.histptr = self.histptr + 1
		self.putoff(buf, pos, CTRL_OFF_EOB, l)
		if l[0] != 0:
			self.putbits(buf, pos, 0, 8-l[0], l)
		self.legacy_in = 0
		
	def update(self, oin):
		out = Octets()
		isize = oin.size()
		pos = [0]
		remain = MPPC_HIST_LEN - self.histptr - self.legacy_in
		if isize >= remain:
			out.resize( (isize + self.legacy_in)*9/8 + 6)
			poso = [0]
			self.history[self.histptr+self.legacy_in:self.histptr+self.legacy_in+remain] \
				= oin.buffer[pos[0]:pos[0]+remain]
			isize = isize - remain
			pos[0] = pos[0] + remain
			self.compress_block(out.buffer, poso, remain+self.legacy_in)
			self.histptr = 0
			while isize >= MPPC_HIST_LEN:
				self.history[self.histptr:self.histptr+MPPC_HIST_LEN] \
					= oin.buffer[pos[0]:pos[0]+MPPC_HIST_LEN]
				self.compress_block(out.buffer, pos, MPPC_HIST_LEN)
				self.histptr = 0
				isize = isize - MPPC_HIST_LEN
				pos[0] = pos[0] + MPPC_HIST_LEN
			out.resize(poso[0])
		self.history[self.histptr+self.legacy_in:self.histptr+self.legacy_in+isize] \
			= oin.buffer[pos[0]:pos[0]+isize]
		self.legacy_in = self.legacy_in + isize
		return oin.swap(out)
	
	def final(self, oin):
		#print 'Compress udpate o:', oin.hexstr()
		if oin.size() == 0 and self.legacy_in == 0: return oin
		out = self.update(oin)
		osize = out.size()
		out.reserve( osize + self.legacy_in * 9 / 8 + 6 )
		for i in range(out.size()): out[i] = '\x00'
		pos = [osize]
		self.compress_block( out.buffer, pos, self.legacy_in)
		out.resize( pos[0] )
		return oin.swap(out)

def lamecopy(arr, dst, src, len):
	if dst - src > 3:
		while True:
			if not len > 3: break
			for i in range(4):
				arr[dst+i] = arr[src+i]
			dst = dst + 4
			src = src + 4
			len = len - 4
	while True:
		if len == 0: break
		len = len - 1
		arr[dst] = arr[src]
		dst = dst + 1
		src = src + 1
		

class Decompress:
	def __init__(self):
		#print 'Decompress __init__', self
		self.history = ['\x00'] * MPPC_HIST_LEN
		self.histptr = 0
		self.l = 0
		self.adjust_l = 0
		self.blen = 0
		self.blen_total = 0
		self.rptr = 0
		self.adjust_rptr = 0
		self.legacy_in = Octets()
	def passbits(self, n):
		self.l = (self.l + n) & 0xffffffffL
		self.blen = (self.blen + n ) & 0xffffffffL
		if self.blen < self.blen_total: return True
		self.l = self.adjust_l
		self.rptr = self.adjust_rptr
		return False
	def fetch(self):
		self.rptr = self.rptr + (( self.l >> 3 ) & 0xffffffffL)
		self.l = self.l & 7
		return 0xffffffffL & (struct.unpack('!I', self.legacy_in.getstr(self.rptr, self.rptr+4))[0] << self.l )
	def __deepcopy__(self, memo):
		d = self.__class__()
		#print 'Decompress __deepcopy__', self , 'to', d
		d.histptr = self.histptr
		d.l = self.l
		d.adjust_l = self.adjust_l
		d.blen = self.blen
		d.blen_total = self.blen_total
		d.rptr = self.rptr
		d.adjust_rptr = self.adjust_rptr
		d.legacy_in = copy.deepcopy(self.legacy_in)
		d.history = self.history[:]
		return d
	def update(self, o):
		#print 'Deompress udpate o:', o.hexstr()
		#print 'Decompress', self, ' o.size is:', o.size()
		#print self.histptr, self.l, self.adjust_l, self.blen, self.blen_total, self.rptr, self.adjust_rptr
		#print self.legacy_in.size()
		#tstr = ''
		#for i in range(o.size()):
		#	tstr = tstr + str(ord(o[i])) + ','
		#print tstr
		self.legacy_in.append(o)
		self.blen_total = ((self.legacy_in.size() * 8) & 0xffffffffL) - self.l
		self.legacy_in.reserve( self.legacy_in.size() + 3 )
		self.rptr = 0
		self.blen = 7
		o.clear()
		histhead = self.histptr
		while self.blen_total > self.blen:
			self.adjust_l = self.l
			self.adjust_rptr = self.rptr
			val = self.fetch()
			if val < 0x80000000L:
				if not self.passbits(8): break
				print self.histptr
				self.history[self.histptr] = chr((val>>24)&0xff)		
				self.histptr = self.histptr + 1
				continue
			if val < 0xc0000000L:
				if not self.passbits(9): break
				print self.histptr
				self.history[self.histptr] = chr((((val>>23)&0xff)|0x80)&0xff)
				self.histptr = self.histptr + 1
				continue
			len = 0
			off = 0
			if val >= 0xf0000000L:
				if not self.passbits(10): break
				off = (val>>22)&0x3f
				if off == CTRL_OFF_EOB:
					advance = 8 - (self.l&7)
					if advance < 8:
						if not self.passbits(advance): break
					o.append(self.history[histhead:self.histptr])
					if self.histptr == MPPC_HIST_LEN:
						self.histptr = 0
					histhead = self.histptr
					continue
			elif val >= 0xe0000000L:
				if not self.passbits(12): break
				off = ((val>>20)&0xff) + 64
			elif val >= 0xc0000000L:
				if not self.passbits(16): break
				off = ((val>>16)&0x1fff) + 320
			val = self.fetch()
			if val < 0x80000000L:
				if not self.passbits(1): break
				len = 3
			elif val < 0xc0000000L:
				if not self.passbits(4): break
				len = 4 | ((val>>28)&3)
			elif val < 0xe0000000L:
				if not self.passbits(6): break
				len = 8 | ((val>>26)&7)
			elif val < 0xf0000000L:
				if not self.passbits(8): break
				len = 16| ((val>>24)&15)
			elif val < 0xf8000000L:
				if not self.passbits(10): break
				len = 32| ((val>>22)&0x1f)
			elif val < 0xfc000000L:
				if not self.passbits(12): break
				len = 64| ((val>>20)&0x3f)
			elif val < 0xfe000000L:
				if not self.passbits(14): break
				len = 128|((val>>18)&0x7f)
			elif val < 0xff000000L:
				if not self.passbits(16): break
				len = 256|((val>>16)&0xff)
			elif val < 0xff800000L:
				if not self.passbits(18): break
				len = 0x200|((val>>14)&0x1ff)
			elif val < 0xffc00000L:
				if not self.passbits(20): break
				len = 0x400|((val>>12)&0x3ff)
			elif val < 0xffe00000L:
				if not self.passbits(22): break
				len = 0x800|((val>>10)&0x7ff)
			elif val < 0xfff00000L:
				if not self.passbits(24): break
				len = 0x1000|((val>>8)&0xfff)
			else:
				self.l = self.adjust_l
				self.rptr = self.adjust_rptr
				break
			if self.histptr < off or self.histptr + len > MPPC_HIST_LEN:
				break
			lamecopy(self.history, self.histptr, self.histptr - off, len)
			self.histptr = self.histptr + len
		o.append(self.history[histhead:self.histptr])
		self.legacy_in.erase(0, self.rptr)
		#print 'after Decompress o.size is:', o.size()
		#print o.hexstr()
		#tstr = ''
		#for i in range(o.size()):
		#	tstr = tstr + str(ord(o[i])) + ','
		#print tstr
		return o


import socket, select, thread, threadpool, security, conf, os

from threadpool import Task
from select import select
#from socket import socket,  AF_INET, SOCK_STREAM
from socket import *
from fcntl import *
from copy import deepcopy
from octets import Octets

print 'import poll'

DEFAULTIOBUF = 8192

class NetSession:
	def __init__(self):
		self.ibuffer = Octets( DEFAULTIOBUF )
		self.obuffer = Octets( DEFAULTIOBUF )
		self.isecbuf = Octets( DEFAULTIOBUF )
		self.isec = security.create('NULLSECURITY')
		self.osec = security.create('NULLSECURITY')
		self.closing = False
		self.locker = thread.allocate_lock()

	def output(self, data):
		if data.size() + self.obuffer.size() > self.obuffer.capacity(): return False
		#print 'before osec, data size:', data.size()
		udata = self.osec.update(data)
		#print 'after osec, data size:', udata.size()
		self.obuffer.append(udata)
		return True
	def sinput(self):
		#print 'netsession sinput', self.ibuffer.size()
		self.locker.acquire()
		try:
			self.ibuffer = self.isec.update(self.ibuffer)
			self.isecbuf.append(self.ibuffer)
			self.ibuffer.clear()
			return self.isecbuf
		finally:
			self.locker.release()

	def setisec(self, _type, key):
		self.locker.acquire()
		try:
			self.isec = security.create(_type)
			self.isec.setparameter(Octets().append(key))
			#print 'session set isec'
		finally:
			self.locker.release()
	def setosec(self, _type, key):
		self.locker.acquire()
		try:
			self.osec = security.create(_type)
			self.osec.setparameter(Octets().append(key))
			#print 'session set osec'
		finally:
			self.locker.release()
	def loadconfig(self):
		section = self.identification()
		self.ibuffer.reserve(int(section['ibuffermax']))
		self.obuffer.reserve(int(section['obuffermax']))
		if section.has_key('isec'):
			self.setisec(section['isec'], section['iseckey'].strip())
		if section.has_key('osec'):
			self.setosec(section['osec'], section['oseckey'].strip())

	def close(self):
		self.closing = True
	
	def onsend(self):
		assert 0 , 'not implemented'
	def onrecv(self):
		assert 0 , 'not implemented'
	def onopen(self):
		assert 0 , 'not implemented'
	def onclose(self):
		assert 0 , 'not implemented'

	def onabort(self):
		return
	def identification(self):
		assert 0 , 'not implemented'
	def oncheckaddress(self, sa):
		return sa

	def __deepcopy__(self, memo):
		s = self.__class__()
		s.ibuffer = Octets(self.ibuffer.capacity())
		s.obuffer = Octets(self.obuffer.capacity())
		s.isecbuf = Octets(self.isecbuf.capacity())
		s.isec = deepcopy(self.isec)
		s.osec = deepcopy(self.osec)
		s.locker = thread.allocate_lock()
		return s

POLL_READ = 0x1
POLL_WRITE = 0x2
POLL_CLOSE = 0x4

class PollIO:
	def __init__(self, sock):
		self.sock = sock
		self.sock.setblocking(0)

	def close(self):
		raise 'not implemented'
	def updateevent(self):
		raise 'not implemented'

	def pollread(self):
		return
	def pollwrite(self):
		return
	def pollexcept(self):
		return

	def readable(self):
		raise 'not implemented'
	def writeable(self):
		raise 'not implemented'

class PollTask(Task):
	def __init__(self):
		Task.__init__(self, 0)
	def run(self):
		poll()
		threadpool.addtask(self)

iomap = {}
ionew = {}
poll_locker = thread.allocate_lock()
new_locker = thread.allocate_lock()

class PollControl(PollIO):
	writer = -1
	reader = -1
	def pollread(self):
		#print 'PollCotrol pollread'
		rst = 256
		try:
			while rst == 256:
				#rst = self.sock.recv(256)
				rst = len(os.read(PollControl.reader, 256))
				if rst == 0:	
					print 'reader read 0'
					self.close()
					return
		except:
			pass
	def __init__(self, r, w):
		PollIO.__init__(self, fromfd(r, AF_INET, SOCK_STREAM))
		PollControl.writer = w
		PollControl.reader = r
		fcntl(PollControl.writer, F_SETFL, fcntl(PollControl.writer, F_GETFL)|os.O_NONBLOCK);
		fcntl(PollControl.reader, F_SETFL, fcntl(PollControl.reader, F_GETFL)|os.O_NONBLOCK);
		#fcntl(w, F_SETFL, fcntl(w, F_GETFL)|O_NONBLOCK)
	def updateevent(self):
		return POLL_READ

def register(pollio):
	new_locker.acquire()
	ionew[pollio.sock] = pollio
	new_locker.release()

pds = os.pipe()
#print pds
register(PollControl(pds[0], pds[1]))

def init():
	threadpool.addtask(PollTask())

def wakeup():
	#print 'before wakeup'
	os.write(PollControl.writer, ' ')
	#print 'after wakeup'
	pass

def poll():
	poll_locker.acquire()
	new_locker.acquire()
	for p in ionew:
		iomap[ionew[p].sock] = ionew[p]
	ionew.clear()
	new_locker.release()
	readsocks = []
	writesocks = []
	removeio = []
	for p, pio in iomap.items():
		event = pio.updateevent()
		if event & POLL_CLOSE:
			pio.close()
			removeio.append(p)
		else:
			if event & POLL_READ: readsocks.append(p)
			if event & POLL_WRITE: writesocks.append(p)
	for p in removeio:
		del iomap[p]
	#print 'before poll, read:', len(readsocks), 'write:', len(writesocks)
	readables, writeables, exceptions = select(readsocks, writesocks, [], 1)
	#print 'after poll, read:', len(readables), 'write:', len(writeables), 'exp:', len(exceptions)
	for p in exceptions:
		iomap[p].pollexcept()
	for p in writeables:
		if p not in exceptions: iomap[p].pollwrite()
	for p in readables:
		if p not in exceptions: iomap[p].pollread()
	poll_locker.release()

class NetIO(PollIO):
	def __init__(self, sock, s):
		PollIO.__init__(self, sock)
		self.session = s

	def close(self):
		self.session.onclose()
		return True

class StreamIO(NetIO):
	def __init__(self, sock, s):
		NetIO.__init__(self, sock, s)
		#print 'StreamIO init, session is:', s, 's.state is:', s.state
		s.onopen()	

	def pollread(self):
		try:
			trysize = self.session.ibuffer.capacity() - self.session.ibuffer.size()
			#print 'try read:', trysize
			reads = self.sock.recv(trysize)
			if len(reads) > 0:
				#print 'read ', len(reads), 'bytes'
				#for x in reads: print ord(x)
				self.session.ibuffer.append(reads)
				return
		except:
			pass
		self.session.locker.acquire()
		try:
			self.session.obuffer.clear()
			self.session.closing = True
		finally:
			self.session.locker.release()
	def pollwrite(self):
		self.session.locker.acquire()
		try:
			ssize = self.sock.send(str(self.session.obuffer))
			#print 'write ', ssize, 'bytes'
			if ssize > 0:
				self.session.obuffer.erase(0, ssize)
			else:
				self.session.obuffer.clear()
				self.session.closing = True
		finally:
			self.session.locker.release()
	def pollexcept(self):
		self.session.locker.acquire()
		try:
			self.session.obuffer.clear()
			self.session.closing = True
		finally:
			self.session.locker.release()

	def updateevent(self):
		event = 0
		if self.session.ibuffer.size() > 0: self.session.onrecv()
		if not self.session.closing: self.session.onsend()
		if self.session.obuffer.size() > 0: event = POLL_WRITE
		if self.session.closing:
			self.sock.close()
			return POLL_CLOSE
		if self.session.ibuffer.size() < self.session.ibuffer.capacity():
			event = event | POLL_READ
		return event

class ActiveIO(PollIO):
	def __init__(self, sock, session):
		PollIO.__init__(self, sock)
		self.asession = session
		self.asession.loadconfig()
		self.closing = False
		wakeup()

	def updateevent(self):
		if self.closing: return POLL_CLOSE
		return POLL_READ | POLL_WRITE

	def pollwrite(self):
		self.closing = True

	def close(self):
		register(StreamIO(self.sock, deepcopy(self.asession)))
		wakeup()

class PassiveIO(PollIO):
	def __init__(self, sock, session):
		PollIO.__init__(self, sock)
		self.asession = session
		self.asession.loadconfig()
		self.closing = False

	def updateevent(self):
		if self.closing: return POLL_CLOSE
		return POLL_READ
	def pollread(self):
		newsock, address = self.sock.accept()
		if newsock: register(StreamIO(newsock, deepcopy(self.asession)))

	def close(self):
		self.closing = True

def open_passive(session):
	section = session.identification()
	_type = section['type']
	if _type == 'tcp':
		myhost = section['address']
		myport = int(section['port'])
		listenbk = int(section['listen_backlog'])
		serversock = socket(AF_INET, SOCK_STREAM)
		#
		if section.has_key('so_sndbuf'):
			serversock.setsockopt(SOL_SOCKET, SO_SNDBUF, int(section['so_sndbuf']))
		if section.has_key('so_rcvbuf'):
			serversock.setsockopt(SOL_SOCKET, SO_RCVBUF, int(section['so_rcvbuf']))
		if section.has_key('tcp_nodelay'):
			serversock.setsockopt(IPPROTO_TCP, TCP_NODELAY, int(section['tcp_nodelay']))
		#
		serversock.bind((myhost, myport))
		serversock.listen(listenbk)
		return register(PassiveIO(serversock, session))
	else:
		return 0

def open_active(session):
	section = session.identification()
	_type = section['type']
	if _type == 'tcp':
		myhost = section['address']
		myport = int(section['port'])
		sock = socket(AF_INET, SOCK_STREAM)
		#
		if section.has_key('so_sndbuf'):
			sock.setsockopt(SOL_SOCKET, SO_SNDBUF, int(section['so_sndbuf']))
		if section.has_key('so_rcvbuf'):
			sock.setsockopt(SOL_SOCKET, SO_RCVBUF, int(section['so_rcvbuf']))
		if section.has_key('tcp_nodelay'):
			sock.setsockopt(IPPROTO_TCP, TCP_NODELAY, int(section['tcp_nodelay']))
		opt = 1
		sock.setsockopt(SOL_SOCKET, SO_KEEPALIVE, opt);
		#
		try:
			sock.connect((myhost, myport))
			return register(ActiveIO(sock, session))
		except:
			print 'active connect error'
			return 0
	else:
		return 0


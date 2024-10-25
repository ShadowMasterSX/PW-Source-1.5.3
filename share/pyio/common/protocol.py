from poll import *
from timer import WatchDog, Observer, addobserver
from octetsstream import OctetsStream
from mymarshal import Marshal, MarshalException
from threadpool import Task
from copy import deepcopy, copy
from state import State
import myparser

print 'import protocol'

xid_count = 0
xid_locker = thread.allocate_lock()

def getstub(name):
	#print 'getstub:', name, 'proto_map', protocol_map.keys()
	#print 'get stub:', name
	return protocol_map[name]


class Stream(OctetsStream):
	def __init__(self, s, size=32):
		OctetsStream.__init__(self, size)
		self.session = s
		self.check_policy = True
		self.checked_size = 0

class ProtocolTask(Task):

	def __init__(self, prior, manager, session, protocol):
		Task.__init__(self, prior)
		self.manager = manager
		self.session = session
		self.protocol = protocol
		self.immediately = False

	def run(self):
		try:
			self.protocol.process(self.manager, self.session)
			#print 'after process'
			if self.session.need_wakeup and not immediately:
				poll.wakeup()
				self.session.need_wakeup = False
		except:
			print 'protocol task run err'
			self.manager.close(self.session)

def dispatch(manager, session, protocol):
	#print 'dispatch ', manager, session, protocol
	prior = manager.priorpolicy(protocol.ptype)
	#print 'prior:', prior
	task = ProtocolTask(prior, manager, session, protocol)
	if prior > 0:
		threadpool.addtask(task)
	else:
		task.immediately = True
		task.run()

class Session(NetSession):
	# Manager manager
	need_wakeup = False
	# State state
	# Stream iss
	# LinkedList os
	# WatchDog timer

	def __init__(self):
		NetSession.__init__(self)
		self.manager = 0
		self.state = 0
		self.iss = 0
		self.os = 0
		self.timer = 0

	def setmanager(self, manager):
		self.manager = manager
		self.state = manager.getinitstate()
		return self

	def __deepcopy__(self, memo):
		s = NetSession.__deepcopy__(self, memo)
		s.state = copy(self.state)
		s.iss = Stream(self)
		s.os = []
		s.timer = WatchDog()
		s.manager = self.manager
		return s

	def identification(self):
		return self.manager.identification()

	def oncheckaddress(self, sa):
		return self.manager.oncheckaddress(sa)

	def onopen(self):
		self.timer.reset()
		self.manager.addsession(self)

	def onclose(self):
		self.manager.delsession(self)

	def onabort(self):
		self.manager.abortsession(self)

	def onrecv(self):
		self.timer.reset()
		self.iss.append(self.sinput())
		self.isecbuf.clear()
		#print 'session onrecv, iss size:', self.iss.size()
		try:
			while True:
				p = decode(self.iss)
				if not p:
					break
				dispatch(self.manager, self, p)
		except ProtocolException:
			print 'protocol decode err'
			self.close()

	def onsend(self):
		#print self, 'onsend, self state is', self.state
		if self.state.timepolicy(self.timer.elapse()):
			if len(self.os) != 0:
				while True:
					o = self.os[0]
					if not self.output(o):
						break
					o = self.os.pop(0)
					if len(self.os) == 0:
						break
				self.timer.reset()
		else:
			self.close()

	def send(self, protocol):
		self.locker.acquire()
		try:
			o = OctetsStream()
			protocol.encode(o)
			if protocol.sizepolicy(o.size()):
				self.os.append(o)
				need_wakeup = True
				return True
			return False
		finally:
			self.locker.release()

	def statepolicy(self, ptype):
		return self.state.typepolicy(ptype)

	def close(self):
		self.closing = True

	def changestate(self, name):
		self.locker.acquire()
		try:
			self.state = state_map[name]
		finally:
			self.locker.release()

class ProtocolException: pass

class Protocol(Marshal):
	#protected int type;
	#protected int size_policy;
	#protected int prior_policy;
	def __init__(self):
		self.ptype = 0
		self.prior_policy = 0
		self.size_policy = 0
	def __deepcopy__(self, memo):
		p = self.__class__()
		p.ptype = self.ptype
		p.prior_policy = self.prior_policy
		p.size_policy =self.size_policy
		return p
	

	def encode(self, os):
		os.compact_uint32(self.ptype).marshalos(OctetsStream().marshal(self))
		#print 'after protocol encode, os size:', os.size()

	def priorpolicy(self):
		return self.prior_policy

	def sizepolicy(self, x):
		return x <= 0 or x < self.size_policy

	def process(self, manager, session):
		assert 0, 'abstract method Protocol::process'

def server(manager):
	return open_passive(Session().setmanager(manager))
def client(manager):
	return open_active(Session().setmanager(manager))

def decode(iss):
	#print 'decode iss'
	if iss.eos(): return 0
	protocol = 0
	try:
		#print 'try decode, iss.check_policy:', iss.check_policy
		if iss.check_policy:
			iss.begin()
			#print 'iss begin'
			ptype = iss.uncompact_uint32()
			size = iss.uncompact_uint32()
			#print 'got protocol, type is:', ptype, 'size is:', size
			iss.rollback()
			if not iss.session.statepolicy(ptype) or \
				not iss.session.manager.inputpolicy(ptype, size):
					print 'raise ProtocolException 1'
					raise ProtocolException()
			iss.check_policy = False
			iss.checked_size = size
		data = Stream(iss.session, iss.checked_size)
		iss.begin()
		ptype = iss.uncompact_uint32()
		#print 'ptype:', ptype
		iss.unmarshal(data)
		iss.commit()
		protocol = create(ptype)
		#print 'in decode datasize is:', data.size()
		if protocol: protocol.unmarshal(data)
		iss.check_policy = True
	except MarshalException:
		print 'raise MarshalException'
		iss.rollback()
		if protocol: raise ProtocolException()
	return protocol

class Rpc(Protocol):
	map = {}
	class HouseKeeper(Observer):
		def __init__(self):
			addobserver(self)
		def update(self):
			for k, v in Rpc.map.items():
				if v.time_policy < v.timer.elapse():
					v.ontimeout()
					del Rpc.map[k]
	housekeeper = HouseKeeper()
	#private XID xid = new XID();
	#private TimerObserver.WatchDog timer = new TimerObserver.WatchDog();
	#protected Data argument;
	#protected Data result;
	#protected long time_policy;
	class XID(Marshal):
		#public int count = 0;
		#private boolean is_request = true;
		def __init__(self):
			self.count = 0
			self.is_request = False
		def marshal(self, os):
			if self.is_request:
				return os.marshal_int32(self.count|0x80000000L)
			return os.marshal_int32(self.count&0x7fffffffL)

		def unmarshal(self, os):
			self.count = os.unmarshal_uint32()
			self.is_request = (self.count & 0x80000000L) != 0
			#print 'unmarshal xid, count :', self.count, ', is_request:', self.is_request
			return os
		def isrequest(self):
			return self.is_request
		def clrrequest(self):
			self.is_request = False
		def setrequest(self):
			import protocol
			self.is_request = True
			protocol.xid_locker.acquire()
			self.count = protocol.xid_count
			#print 'set request, count:', self.count
			protocol.xid_count = protocol.xid_count+1
			protocol.xid_locker.release()
		def __deepcopy__(self, memo):
			newxid = self.__class__()
			newxid.count = self.count
			newxid.is_request = self.is_request
			#print 'deepcopy xid'
			return newxid
		def __eq__(self, o):
			return self.count &0x7fffffffL == o.count &0x7fffffffL
		def __hash__(self):
			return int(self.count & 0x7fffffffL)
	
	def marshal(self, os):
		os.marshal(self.xid)
		if self.xid.isrequest():
			os.marshal(self.argument)
		else:
			#print 'before marshal result, result is:', self.result, 'os size:', os.size()
			os.marshal(self.result)
		#print 'before rpc marshal return, os.size():', os.size(), 'os.pos:', os.pos
		return os
	def unmarshal(self, os):
		os.unmarshal(self.xid)
		if self.xid.isrequest(): return os.unmarshal(self.argument)
		rpc = Rpc.map[self.xid]
		if rpc: os.unmarshal(rpc.result)
		return os

	def __init__(self):
		Protocol.__init__(self)
		self.xid = Rpc.XID()
		self.timer = 0
		self.time_policy = 0
		self.argument = 0
		self.result = 0

	def __deepcopy__(self, memo):
		rpc = Protocol.__deepcopy__(self, memo)
		rpc.xid = deepcopy(self.xid)
		rpc.timer = WatchDog()
		rpc.argument = deepcopy(self.argument)
		rpc.result = deepcopy(self.result)
		return rpc

	def process(self, manager, session):
		if self.xid.isrequest():
			self.server(self.argument, self.result, manager, session)
			self.xid.clrrequest()
			manager.send(session, self)
			return
		if Rpc.map.has_key(self.xid):
			p = Rpc.map[self.xid]
			p.client(p.argument, p.result)
			del Rpc.map[self.xid]
	def server(self, argument, result, manager, session):
		pass
	def client(self, argument, result):
		pass
	def ontimeout(self):
		pass

class Manager:

	def __init__(self):
		self.set = {}
		self._section = 0
	
	def addsession(self, session):
		self.set[session] = 1
		self.onaddsession(session)

	def delsession(self, session):
		self.ondelsession(session)
		if self.set.has_key(session): del self.set[session]

	def abortsession(self, session):
		self.onabortsession(session)

	def setisecurity(self, session, _type, key):
		if not self.set.has_key(session): return False
		session.setisec(_type, key)
		return True

	def setosecurity(self, session, _type, key):
		if not self.set.has_key(session): return False
		session.setosec(_type, key)
		return True
	def load_section(self, str):
		self._section = conf.get_sec(str)
		assert self._section != 0, 'manager load section'

	def send(self, session, protocol):
		if not self.set.has_key(session): return False
		return session.send(protocol)

	def close(self, session):
		if not self.set.has_key(session): return False
		session.close()
		return True
	
	def changestate(self, session, name):
		if not self.set.has_key(session): return False
		session.changestate(name)
		return True

	def onaddsession(self, session):
		assert 0, 'abstract method Manager::onaddsession'

	def ondelsession(self, session):
		assert 0, 'abstract method Manager::ondelsession'

	def onabortsession(self, session):
		pass

	def getinitstate(self):
		assert 0, 'abstract method Manager::getinitstate'

	def priorpolicy(self, _type):
		return getstub(_type).priorpolicy()
	def inputpolicy(self, _type, size):
		return getstub(_type).sizepolicy(size)

	def identification(self):
		assert 0, 'abstract method Manager::identification'

	def oncheckaddress(self, sa):
		return sa

state_map = myparser.parsestate()
#print state_map
protocol_map = myparser.parseprotocol()
#print protocol_map
	
def create(name):
	rpc = getstub(name)
	#if not stub: return 0
	#print 'in create, rpc is:', rpc
	return deepcopy(rpc)

def call(_type, arg):
	rpc = create(_type)
	rpc.xid.setrequest()
	rpc.argument = arg
	Rpc.map[rpc.xid] = rpc
	return rpc

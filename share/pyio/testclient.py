from state import State
import protocol, poll
from protocol import *
import threading

class TestClient(Manager):
	def __init__(self):
		Manager.__init__(self)
		self.session = 0
		self.locker = threading.Condition()
	def onaddsession(self, session):
		self.locker.acquire()
		try:
			self.session = session
			self.locker.notifyAll()
			print 'TestClient onaddsession', str(session)
		finally:
			self.locker.release()
	def ondelsession(self, session):
		self.locker.acquire()
		try:
			self.session = 0
			self.locker.notifyAll()
			print 'TestClient ondelsession', str(session)
		finally:
			self.locker.release()
	def onabortsession(self, session):
		self.locker.acquire()
		try:
			self.session = 0
			self.locker.notifyAll()
			print 'TestClient onabortsession', str(session)
		finally:
			self.locker.release()
	def getinitstate(self):
		return state_map['normal']
	def identification(self):
		return self._section
	def load_section(self):
		return self._section
	def tryconnect(self):
		self.locker.acquire()
		try:
			while self.session == 0:
				if 0 ==protocol.client(self): return -1
				wakeup()
				self.locker.wait()
				return 0
		finally:
			self.locker.release()

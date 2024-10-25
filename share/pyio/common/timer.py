from time import time
import usertimer

print 'import timer'

now = time()
obs = {}

class Observer:
	def update(self):
		assert 0, 'abstract method Observer::update'

def addobserver(ob):
	obs[ob] = 1

def notifyobservers():
	for ob in obs.keys():
		ob.update()

def timer_update():
	#do update here
	now = time()
	notifyobservers()
	usertimer.update()

class WatchDog:
	def __init__(self):
		self.t = now
	def reset(self):
		self.t = now
	def elapse(self):
		return now - self.t

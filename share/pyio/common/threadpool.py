
import conf
import re
import thread
import threading
import time
import alarm

print 'import threadpool'

class Task:
	def __init__(self, prior):
		self.prior = prior
	def run(self):
		raise "not implement"
	def get_prior(self):
		return self.prior

bStop = False
tasks = {}
task_count = 0
locker = threading.Condition()

def init():
	thread_config = conf.get_conf('ThreadPool', 'threads')
	if thread_config == '':
		thread_config = conf.get_conf('ThreadPool', 'config')
	if thread_config == '': raise 'thread config not found'
	pobj = re.compile('\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\)')
	mobj = True
	while mobj:
		mobj = pobj.match(thread_config)
		if mobj:
			p = mobj.group(1)
			c = mobj.group(2)
			for i in range(int(c)):
				thread.start_new(thread_run, (p,))
			thread_config = thread_config[mobj.end():]

def thread_run(prior):
	global task_count
	print 'thread with prior', prior, 'start'
	while True:
		locker.acquire()
		while not bStop and 0 == task_count: locker.wait()
		if not bStop: 
			if prior in tasks and len(tasks[prior]) != 0:
				r = tasks[prior].pop(0)
			else:
				for q in tasks:
					if len(tasks[q]) != 0:
						r = tasks[q].pop(0)
						break
			task_count = task_count - 1
			locker.release()
			r.run()
		else:
			locker.release()
			break
	print 'thread with prior', prior, 'leave'

def get_taskcount():
	return task_count

def addtask(task):
	global task_count
	locker.acquire()
	prior = task.get_prior()
	if prior not in tasks:
		tasks[prior] = [task]
	else:
		tasks[prior].append(task)
	task_count = task_count + 1
	locker.notify()
	locker.release()

def stop():
	global bStop
	locker.acquire()
	bStop = True
	locker.notifyAll()
	locker.release()

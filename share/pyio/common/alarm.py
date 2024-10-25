import timer, threading

print 'import alarm'

def alarm_hander():
	timer.timer_update()
	t = threading.Timer(1, alarm_hander)
	t.start()

t = threading.Timer(1, alarm_hander)
t.start()

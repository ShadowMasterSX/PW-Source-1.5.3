#!/usr/bin/python

import sys, struct, threading, time
COMMON_PATH = 'common/'
sys.path.append(COMMON_PATH)

import conf, copy
import threadpool, poll, protocol
from testclient import TestClient
from stub import *

try:
	conf.load_conf('io.conf')
	threadpool.init()
	poll.init()

	ips = ['172.16.2.117', '172.16.2.116']
	for i in range(len(ips)):
		tc = TestClient()
		tc._section = conf.clone_sec('TestClient')
		tc._section['address'] = ips[i]
		print 'client ', i , 'with address', tc._section['address'], tc._section['port']

		if -1 != tc.tryconnect():
			print 'connected'
			targ = GetLinePlayerLimitArg()
			rpc = protocol.call('GetLinePlayerLimit', targ)
			tc.send(tc.session, rpc)
			#qclient.session.close()
		else:
			print 'client connect failed'

	while True:
		time.sleep(1)
except:
	raise
	sys.exit(1)








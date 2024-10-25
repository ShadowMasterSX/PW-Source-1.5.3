
def rpcdata_GetLinePlayerLimitArg_init_ext(self):
	pass

def rpcdata_LinePlayerNumberLimits_init_ext(self):
	pass

def rpcdata_LinePlayerLimit_init_ext(self):
	pass

def rpcdata_IntData_init_ext(self):
	pass

def rpcdata_IntOctets_init_ext(self):
	pass

def rpcdata_SetLinePlayerLimitRes_init_ext(self):
	pass

def rpc_SetLinePlayerLimit_init_ext(self):
	pass

def rpc_SetLinePlayerLimit_server(self, argument, result, manager, session):
	pass

def rpc_SetLinePlayerLimit_client(self, argument, result):
	pass

def rpc_SetLinePlayerLimit_timeout(self):
	pass

def rpc_GetLinePlayerLimit_init_ext(self):
	pass

def rpc_GetLinePlayerLimit_server(self, argument, result, manager, session):
	pass

def rpc_GetLinePlayerLimit_client(self, argument, result):
	print 'client of getlineplayerlimit'
	print len(self.result.limits)

def rpc_GetLinePlayerLimit_timeout(self):
	pass

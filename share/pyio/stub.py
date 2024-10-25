#6	rpcdatas
#0	protocols
#2	rpcs
from copy import deepcopy
from octets import Octets
from action import *
from rpcdata import *
from protocol import Protocol, Rpc
from mymarshal import Marshal

print 'import stub'

class GetLinePlayerLimitArg(Marshal):
	def __init__(self):
		self.reserved	=	Int(0)
		rpcdata_GetLinePlayerLimitArg_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	self.__class__()
		_newobj.reserved	=	deepcopy(self.reserved)
		return _newobj
	def marshal(self, os):
		self.reserved.marshal(os)
		return os
	def unmarshal(self, os):
		self.reserved	=	Int(os.unmarshal_int32())
		return os


class LinePlayerNumberLimits(Marshal):
	def __init__(self):
		self.limits	=	List('LinePlayerLimit')
		rpcdata_LinePlayerNumberLimits_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	self.__class__()
		_newobj.limits	=	deepcopy(self.limits)
		return _newobj
	def marshal(self, os):
		self.limits.marshal(os)
		return os
	def unmarshal(self, os):
		self.limits.unmarshal(os)
		return os


class LinePlayerLimit(Marshal):
	def __init__(self):
		self.line_id	=	Int(0)
		self.cur_num	=	Int(0)
		self.max_num	=	Int(1)
		rpcdata_LinePlayerLimit_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	self.__class__()
		_newobj.line_id	=	deepcopy(self.line_id)
		_newobj.cur_num	=	deepcopy(self.cur_num)
		_newobj.max_num	=	deepcopy(self.max_num)
		return _newobj
	def marshal(self, os):
		self.line_id.marshal(os)
		self.cur_num.marshal(os)
		self.max_num.marshal(os)
		return os
	def unmarshal(self, os):
		self.line_id	=	Int(os.unmarshal_int32())
		self.cur_num	=	Int(os.unmarshal_int32())
		self.max_num	=	Int(os.unmarshal_int32())
		return os


class IntData(Marshal):
	def __init__(self):
		self.int_value	=	Int(0)
		rpcdata_IntData_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	self.__class__()
		_newobj.int_value	=	deepcopy(self.int_value)
		return _newobj
	def marshal(self, os):
		self.int_value.marshal(os)
		return os
	def unmarshal(self, os):
		self.int_value	=	Int(os.unmarshal_int32())
		return os


class IntOctets(Marshal):
	def __init__(self):
		self.m_int	=	Int(-1)
		self.m_octets	=	Octets()
		rpcdata_IntOctets_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	self.__class__()
		_newobj.m_int	=	deepcopy(self.m_int)
		_newobj.m_octets	=	deepcopy(self.m_octets)
		return _newobj
	def marshal(self, os):
		self.m_int.marshal(os)
		self.m_octets.marshal(os)
		return os
	def unmarshal(self, os):
		self.m_int	=	Int(os.unmarshal_int32())
		self.m_octets.unmarshal(os)
		return os


class SetLinePlayerLimitRes(Marshal):
	def __init__(self):
		self.retcode	=	Char(-1)
		rpcdata_SetLinePlayerLimitRes_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	self.__class__()
		_newobj.retcode	=	deepcopy(self.retcode)
		return _newobj
	def marshal(self, os):
		self.retcode.marshal(os)
		return os
	def unmarshal(self, os):
		self.retcode	=	Char(os.unmarshal_int8())
		return os


class SetLinePlayerLimit(Rpc):
	def __init__(self):
		Rpc.__init__(self)
		rpc_SetLinePlayerLimit_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	Rpc.__deepcopy__(self, memo)
		return _newobj

	def server(self, argument, result, manager, session):
		rpc_SetLinePlayerLimit_server(self, argument, result, manager, session)
	def client(self, argument, result):
		rpc_SetLinePlayerLimit_client(self, argument, result)
	def timeout(self):
		rpc_SetLinePlayerLimit_timeout(self)

class GetLinePlayerLimit(Rpc):
	def __init__(self):
		Rpc.__init__(self)
		rpc_GetLinePlayerLimit_init_ext(self)
	def __deepcopy__(self, memo):
		_newobj	=	Rpc.__deepcopy__(self, memo)
		return _newobj

	def server(self, argument, result, manager, session):
		rpc_GetLinePlayerLimit_server(self, argument, result, manager, session)
	def client(self, argument, result):
		rpc_GetLinePlayerLimit_client(self, argument, result)
	def timeout(self):
		rpc_GetLinePlayerLimit_timeout(self)


import xml.dom.minidom
from xml.dom.minidom import Node
from state import State
import stat, os
#from protocol import Protocol
import re

print 'import myparser'

def getchildnodes(pnode, childname):
	allnodes = pnode.getElementsByTagName(childname)
	r = []
	for n in allnodes:
		if n.nodeType == Node.ELEMENT_NODE:
			r.append(n)
	return r

xmlfile = 'rpcalls.xml'
stub_py = 'stub.py'
action_py = 'action.py'

dom = xml.dom.minidom.parseString(open(xmlfile, 'r').read())
maintypes = {}
maintypes['int'] = 'Int'
maintypes['unsigned int'] = 'UInt'
maintypes['byte'] = 'Char'
maintypes['Byte'] = 'UChar'
maintypes['char'] = 'Char'
maintypes['unsigned char'] = 'UChar'
maintypes['short'] = 'Short'
maintypes['unsigend short'] = 'UShort'
maintypes['long'] = 'Long'
maintypes['unsigned long'] = 'ULong'
maintypes['float'] = 'Float'
maintypes['double'] = 'Double'

unmarshalmap = {}
unmarshalmap['Int'] = 'int32'
unmarshalmap['UInt'] = 'uint32'
unmarshalmap['Char'] = 'int8'
unmarshalmap['UChar'] = 'uint8'
unmarshalmap['Short'] = 'int16'
unmarshalmap['UShort'] = 'uint16'
unmarshalmap['Long'] = 'int64'
unmarshalmap['ULong'] = 'uint64'
unmarshalmap['Float'] = 'float'
unmarshalmap['Double'] = 'double'

def gettype(name):
	v = False
	if name.endswith('Vector'):
		v = True
		name = name[0:len(name)-6]
	if maintypes.has_key(name):
		name = maintypes[name]
	if v:
		return 'List(\'' + name + '\')'
	return name+'()'

def parsestate():
	map = {}

	protos = dom.getElementsByTagName('protocol')
	rpcs = dom.getElementsByTagName('rpc')
	rpcs.extend(protos)
	rpc_map = {}
	for rpc in rpcs:
		attrs = rpc.attributes
		if attrs.has_key('name') and attrs.has_key('type'):
			rpc_map[attrs['name'].value] = attrs['type'].value
	states = dom.getElementsByTagName('state')
	for state in states:
		if state.nodeType != Node.ELEMENT_NODE or not state.hasChildNodes(): continue
		attrs = state.attributes
		if not attrs.has_key('name') or not attrs.has_key('timeout'): continue
		stateobj = State(int(attrs['timeout'].value))
		protos = state.childNodes
		for proto in protos:
			if proto.nodeType != Node.ELEMENT_NODE: continue
			if proto.attributes.has_key('name'):
				pname = proto.attributes['name'].value
				if rpc_map.has_key(pname):
					stateobj.addprotocoltype(rpc_map[pname])
		map[attrs['name'].value] = stateobj
	return map

def parseprotocol():
	import stub
	r = {}
	protocols = getchildnodes(dom, 'protocol')
	for proto in protocols:
		attrs = proto.attributes
		name = attrs['name'].value
		ptype = attrs['type'].value
		maxsize = attrs['maxsize'].value
		if attrs.has_key('prior'):
			prior = attrs['prior'].value
		else:
			prior = attrs['priority'].value
		print 'proto:', name, ptype, maxsize, prior
		protoobj = eval('stub.' + name +'()')
		protoobj.ptype = int(ptype)
		protoobj.size_policy = int(maxsize)
		protoobj.prior_policy = int(prior)
		r[name] = protoobj
		r[int(ptype)] = protoobj
	rpcs = getchildnodes(dom, 'rpc')
	for rpc in rpcs:
		attrs = rpc.attributes
		name = attrs['name'].value
		ptype = attrs['type'].value
		maxsize = attrs['maxsize'].value
		if attrs.has_key('prior'):
			prior = attrs['prior'].value
		else:
			prior = attrs['priority'].value
		timeout = attrs['timeout'].value
		argument = attrs['argument'].value
		result = attrs['result'].value
		print 'rpc:', name, ptype, maxsize, prior, timeout
		protoobj = eval('stub.' + name +'()')
		protoobj.ptype = int(ptype)
		protoobj.size_policy = int(maxsize)
		protoobj.prior_policy = int(prior)
		protoobj.time_policy = int(timeout)

		protoobj.argument = eval('stub.' + gettype(argument))
		protoobj.result = eval('stub.' +gettype(result))

		r[name] = protoobj
		r[int(ptype)] = protoobj
	return r

def geninitandmarshal(rpcdata_map, nodename, node, nodetype, action_functions, newactionlines):
	lines = []
	vars = getchildnodes(node, 'variable')
	var_map = []
	for var in vars:
		attrs = var.attributes
		if attrs.has_key('name'):
			var_name = attrs['name'].value
			if var_name == 'type': var_name = '_type'
			var_map.append([var_name, var, 0])
	lines.append('\tdef __init__(self):\n')
	if nodetype == 'rpc':
		lines.append('\t\tRpc.__init__(self)\n')
	elif nodetype == 'protocol':
		lines.append('\t\tProtocol.__init__(self)\n')
	has_var = False
	for varpair in var_map:
		name = varpair[0]
		var = varpair[1]
		attrs = var.attributes	
		if not attrs.has_key('type'):continue
		ptype = attrs['type'].value
		rtype = gettype(ptype)
		varpair[2] = rtype[0:len(rtype)-2]
		default = ''
		if attrs.has_key('default'): default = attrs['default'].value
		line = '\t\tself.' + name + '\t=\t'
		if not default == '':
			if default.find('(') == -1:
				stype = gettype(ptype)
				stype = stype[0:len(stype)-1] + default + ')'
				line = line + stype
			else:
				line = line + rtype
		else:
			line = line + rtype
		line = line + '\n'
		lines.append(line)
		has_var = True
	ext_init_name = nodetype + '_' + nodename + '_init_ext'
	lines.append('\t\t' + ext_init_name + '(self)\n')
	if not action_functions.has_key(ext_init_name):
		newactionlines.append('\n')
		newactionlines.append('def ' + ext_init_name + '(self):\n')
		newactionlines.append('\tpass\n')

	lines.append('\tdef __deepcopy__(self, memo):\n')
	if nodetype == 'rpcdata':
		lines.append('\t\t_newobj\t=\tself.__class__()\n')
	elif nodetype == 'rpc':
		lines.append('\t\t_newobj\t=\tRpc.__deepcopy__(self, memo)\n')
	else:
		lines.append('\t\t_newobj\t=\tProtocol.__deepcopy__(self, memo)\n')
	for var_name in var_map:
		lines.append('\t\t_newobj.' + var_name[0] + '\t=\tdeepcopy(self.' + var_name[0] + ')\n')
	lines.append('\t\treturn _newobj\n')

	if nodetype == 'rpcdata' or nodetype == 'protocol':
		lines.append('\tdef marshal(self, os):\n')
		for varpair in var_map:
			lines.append('\t\tself.' + varpair[0] + '.marshal(os)\n')
		if not has_var:
			lines.append('\t\tpass\n')
		else:
			lines.append('\t\treturn os\n')
		lines.append('\tdef unmarshal(self, os):\n')
		for varpair in var_map:
			rtype = varpair[2]
			if unmarshalmap.has_key(rtype):
				lines.append('\t\tself.' + varpair[0] + '\t=\t' + rtype + \
					'(os.unmarshal_' + unmarshalmap[rtype] + '())\n')
			else:
				lines.append('\t\tself.' + varpair[0] + '.unmarshal(os)\n')
		if not has_var:
			lines.append('\t\tpass\n')
		else:
			lines.append('\t\treturn os\n')

	lines.append('\n')
	return lines

def genrpcdata(rpcdata_map, action_functions, newactionlines):
	lines = []
	for name, data in rpcdata_map.items():
		lines.append('class ' + name + '(Marshal):\n')
		lines.extend(geninitandmarshal(rpcdata_map, name, data, 'rpcdata', action_functions, newactionlines))
		lines.append('\n')
	return lines

def genmanager():
	managers = getchildnodes(dom, 'manager')
	for manager in managers:
		attrs = manager.attributes
		name = attrs['name'].value
		mtype = attrs['type'].value.lower()
		ident = attrs['identification'].value
		istate = attrs['initstate'].value
		filename = name.lower() + '.py'
		if os.access(filename, os.F_OK): continue
		lines = []
		if mtype == 'server':
			lines.append('from state import State\n')
			lines.append('import protocol, poll\n')
			lines.append('from protocol import *\n')
			lines.append('\n')
			lines.append('class ' + name + '(Manager):\n')
			lines.append('\tdef __init__(self):\n')
			lines.append('\t\tManager.__init__(self)\n')
			lines.append('\tdef onaddsession(self, session):\n')
			lines.append('\t\tprint \'' + name + ' onaddsession\', str(session)\n')
			lines.append('\tdef ondelsession(self, session):\n')
			lines.append('\t\tprint \'' + name + ' ondelsession\', str(session)\n')
			lines.append('\tdef getinitstate(self):\n')
			lines.append('\t\treturn state_map[\'' + istate + '\']\n')
			lines.append('\tdef identification(self):\n')
			lines.append('\t\treturn \'' + ident + '\'\n')
			lines.append('\tdef trylisten(self):\n')
			lines.append('\t\tprotocol.server(self)\n')
			lines.append('\t\twakeup()\n')
		elif mtype == 'client':
			lines.append('from state import State\n')
			lines.append('import protocol, poll\n')
			lines.append('from protocol import *\n')
			lines.append('import threading\n')
			lines.append('\n')
			lines.append('class ' + name + '(Manager):\n')
			lines.append('\tdef __init__(self):\n')
			lines.append('\t\tManager.__init__(self)\n')
			lines.append('\t\tself.session = 0\n')
			lines.append('\t\tself.locker = threading.Condition()\n')
			lines.append('\tdef onaddsession(self, session):\n')
			lines.append('\t\tself.locker.acquire()\n')
			lines.append('\t\ttry:\n')
			lines.append('\t\t\tself.session = session\n')
			lines.append('\t\t\tself.locker.notifyAll()\n')
			lines.append('\t\t\tprint \'' + name + ' onaddsession\', str(session)\n')
			lines.append('\t\tfinally:\n')
			lines.append('\t\t\tself.locker.release()\n')

			lines.append('\tdef ondelsession(self, session):\n')
			lines.append('\t\tself.locker.acquire()\n')
			lines.append('\t\ttry:\n')
			lines.append('\t\t\tself.session = 0\n')
			lines.append('\t\t\tself.locker.notifyAll()\n')
			lines.append('\t\t\tprint \'' + name + ' ondelsession\', str(session)\n')
			lines.append('\t\tfinally:\n')
			lines.append('\t\t\tself.locker.release()\n')

			lines.append('\tdef onabortsession(self, session):\n')
			lines.append('\t\tself.locker.acquire()\n')
			lines.append('\t\ttry:\n')
			lines.append('\t\t\tself.session = 0\n')
			lines.append('\t\t\tself.locker.notifyAll()\n')
			lines.append('\t\t\tprint \'' + name + ' onabortsession\', str(session)\n')
			lines.append('\t\tfinally:\n')
			lines.append('\t\t\tself.locker.release()\n')

			lines.append('\tdef getinitstate(self):\n')
			lines.append('\t\treturn state_map[\'' + istate + '\']\n')
			lines.append('\tdef identification(self):\n')
			lines.append('\t\treturn self._section\n')
			lines.append('\tdef load_section(self):\n')
			lines.append('\t\treturn self._section\n')

			lines.append('\tdef tryconnect(self):\n')
			lines.append('\t\tself.locker.acquire()\n')
			lines.append('\t\ttry:\n')
			lines.append('\t\t\twhile self.session == 0:\n')
			lines.append('\t\t\t\tif 0 ==protocol.client(self): return -1\n')
			lines.append('\t\t\t\twakeup()\n')
			lines.append('\t\t\t\tself.locker.wait()\n')
			lines.append('\t\t\t\treturn 0\n')
			lines.append('\t\tfinally:\n')
			lines.append('\t\t\tself.locker.release()\n')

		mfile = open(filename, 'w')
		mfile.writelines(lines)
		mfile.close()

def genprotocol(proto_map, rpcdata_map, action_functions, newactionlines):
	lines = []
	for name, proto in proto_map.items():
		lines.append('class ' + name + '(Protocol):\n')
		lines.extend(geninitandmarshal(rpcdata_map, name, proto, 'protocol', action_functions, newactionlines))

		lines.append('\tdef process(self, manager, session):\n')
		process_func_name = 'protocol_' + name + '_process'
		lines.append('\t\t' + process_func_name + '(self, manager, session)\n')
		if not action_functions.has_key(process_func_name):
			newactionlines.append('\n')
			newactionlines.append('def ' + process_func_name + '(self, manager, session):\n')
			newactionlines.append('\tpass\n')

		lines.append('\n')
	return lines

def genrpc(rpc_map, rpcdata_map, action_functions, newactionlines):
	lines = []
	for name, rpc in rpc_map.items():
		lines.append('class ' + name + '(Rpc):\n')
		lines.extend(geninitandmarshal(rpcdata_map, name, rpc, 'rpc', action_functions, newactionlines))

		lines.append('\tdef server(self, argument, result, manager, session):\n')
		server_func_name = 'rpc_' + name + '_server'
		lines.append('\t\t' + server_func_name + '(self, argument, result, manager, session)\n')
		if not action_functions.has_key(server_func_name):
			newactionlines.append('\n')
			newactionlines.append('def ' + server_func_name + '(self, argument, result, manager, session):\n')
			newactionlines.append('\tpass\n')
		lines.append('\tdef client(self, argument, result):\n')
		client_func_name = 'rpc_' + name + '_client'
		lines.append('\t\t' + client_func_name + '(self, argument, result)\n')
		if not action_functions.has_key(client_func_name):
			newactionlines.append('\n')
			newactionlines.append('def ' + client_func_name + '(self, argument, result):\n')
			newactionlines.append('\tpass\n')
		lines.append('\tdef timeout(self):\n')
		timeout_func_name = 'rpc_' + name + '_timeout'
		lines.append('\t\t' + timeout_func_name + '(self)\n')
		if not action_functions.has_key(timeout_func_name):
			newactionlines.append('\n')
			newactionlines.append('def ' + timeout_func_name + '(self):\n')
			newactionlines.append('\tpass\n')

		lines.append('\n')
	return lines

def parsestub():
	actionfile = open(action_py, 'a+')
	actionlines = actionfile.readlines()
	action_functions = {}
	pobj = re.compile(r'^def (\w+)\(')
	for line in actionlines:
		mobj = pobj.match(line)
		if mobj:
			action_functions[mobj.group(1)] = 1
	newactionlines = []
	lines = []
	lines.append('from copy import deepcopy\n')
	lines.append('from octets import Octets\n')
	lines.append('from action import *\n')
	lines.append('from rpcdata import *\n')
	lines.append('from protocol import Protocol, Rpc\n')
	lines.append('from mymarshal import Marshal\n')
	lines.append('\n')
	lines.append('print \'import stub\'\n')
	lines.append('\n')
	rpcdatas = getchildnodes(dom, 'rpcdata')
	rpcdata_map = {}
	for rpcdata in rpcdatas:
		attrs = rpcdata.attributes
		if attrs.has_key('name'):
			rpcdata_name = attrs['name'].value
			rpcdata_map[rpcdata_name] = rpcdata
	lines.extend(genrpcdata(rpcdata_map, action_functions, newactionlines))

	protocols = getchildnodes(dom, 'protocol')
	proto_map = {}
	for proto in protocols:
		attrs = proto.attributes
		if attrs.has_key('name'):
			proto_name = attrs['name'].value
			proto_map[proto_name] = proto
	lines.extend(genprotocol(proto_map, rpcdata_map, action_functions, newactionlines))

	rpcs = getchildnodes(dom, 'rpc')
	rpc_map = {}
	for rpc in rpcs:
		attrs = rpc.attributes
		if attrs.has_key('name'):
			rpc_name = attrs['name'].value
			rpc_map[rpc_name] = rpc
	lines.extend(genrpc(rpc_map, rpcdata_map, action_functions, newactionlines))

	commentlines = []
	commentlines.append('#' + str(len(rpcdata_map)) + '\trpcdatas\n')
	commentlines.append('#' + str(len(proto_map)) + '\tprotocols\n')
	commentlines.append('#' + str(len(rpc_map)) + '\trpcs\n')
	commentlines.extend(lines)

	stubfile = open(stub_py, 'w')
	stubfile.writelines(commentlines)
	stubfile.close()
	actionfile.writelines(newactionlines)
	actionfile.close()
	genmanager()

#
if not os.access(stub_py, os.F_OK) or os.stat(xmlfile)[stat.ST_MTIME] > os.stat(stub_py)[stat.ST_MTIME]:
	print 'generating stubs ...'
	parsestub()
else:
	print 'stubs is up to date'
#


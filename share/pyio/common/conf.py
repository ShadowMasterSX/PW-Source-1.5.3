
print 'import conf'

table = {}

def clear():
	table.clear()

def dump():
	for secname in table:
		print 'secname: ' + secname
		sec = table[secname]
		for name in sec: print '\t' + name + '=\t' + sec[name]

def load_conf(conffile):
	cf = open(conffile, 'r')
	lines = cf.readlines()
	secnamenow = ''
	secnow = {}
	for x in lines:
		line = x.strip()
		if len(line) == 0 or line[0] == '#': continue
		if line[0] == '[':
			r = line.find(']')
			if r > 0:
				newsecname = line[1:r].strip()
				if len(newsecname) != 0:
					if secnamenow != '': table[secnamenow] = secnow
					secnamenow = newsecname		
					secnow = {}
		elif secnamenow != '':
			a = line.split('=')
			if len(a) == 2:
				name = a[0].strip()
				value = a[1].strip()
				if len(name) != 0 and len(value) != 0: secnow[name] = value
	if secnamenow != '': table[secnamenow] = secnow

def get_conf(sec, name):
	if sec not in table: return ''
	tt = table[sec]
	if name not in tt: return ''
	return tt[name]

def get_sec(sec):
	if sec not in table: return 0
	return table[sec]

def clone_sec(sec):
	if sec not in table: return 0
	new_sec = {}
	for k,v in table[sec].items():
		new_sec[k] = v
	return new_sec
	

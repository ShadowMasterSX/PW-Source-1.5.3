
TOP_SRCDIR = ..

SINGLE_THREAD = true
DEBUG_VERSION = true

include ../mk/gcc.defs.mk

INCLUDES += -I$(TOP_SRCDIR)/logclient -I$(TOP_SRCDIR)/log_inl -I$(TOP_SRCDIR)/include
DEFINES += -DUSE_HASH_MAP -DUSE_LOGCLIENT  
DEFINES += -D__USER__=\"$(USER)\"
LDFLAGS += -DUSE_HASH_MAP -L/usr/local/ssl/lib -L/usr/lib

OUTEROBJS = $(CO_DIR)/matcher.o trade.o ./mailbox.o ./postoffice.o ./auctionmarket.o ./serverattr.o ./pshopmarket.o
CLEAN += $(OUTEROBJS)

#OBJS = map.o gdeliveryserver.o gauthclient.o gproviderserver.o ganticheatclient.o uniquenameclient.o state.o stubs.o \
#	   gdeliveryd.o gamedbclient.o gfactionclient.o gmrestartserver.o groledbclient.o chatroom.o battlemanager.o \
#	   domaindataman.o countrybattleman.o playerlogin.o maptaskdata.o stockexchange.o billingagent.o domaindaemon.o gamemaster.o \
#	   referencemanager.o refspreadcode.o rewardmanager.o gwebtradeclient.o webtrademarket.o cert.o sysauctionmanager.o \
#	   factionfortressmanager.o gametalkmanager.o gametalkclient.o snsclient.o snsmanager.o forcemanager.o	\
#	   friendextinfomanager.o discountman.o playerlogin_re.o globalcontrol.o disabled_system.o centraldeliveryclient.o centraldeliveryserver.o crosssystem.o \
#	   namemanager.o kingelection.o gdeliverytool.o playerprofileman.o autoteamman.o uniquedataserver.o tankbattlemanager.o factionresourcebattleman.o mappasswd.o
#9.3 reverse
OBJS =  mappasswd.o factionresourcebattleman.o  tankbattlemanager.o  uniquedataserver.o  autoteamman.o  playerprofileman.o  gdeliverytool.o  kingelection.o  namemanager.o  crosssystem.o  centraldeliveryserver.o  centraldeliveryclient.o  disabled_system.o  globalcontrol.o  playerlogin_re.o  discountman.o  friendextinfomanager.o  forcemanager.o  snsmanager.o  snsclient.o  gametalkclient.o  gametalkmanager.o  factionfortressmanager.o  sysauctionmanager.o  webtrademarket.o  gwebtradeclient.o  rewardmanager.o  refspreadcode.o  referencemanager.o  gamemaster.o billingagent.o  stockexchange.o  maptaskdata.o  playerlogin.o  countrybattleman.o  domaindataman.o  battlemanager.o  chatroom.o  groledbclient.o  gmrestartserver.o  gfactionclient.o  gamedbclient.o  gdeliveryd.o  stubs.o  state.o  uniquenameclient.o  ganticheatclient.o  gproviderserver.o  gauthclient.o  gdeliveryserver.o map.o 	   

all : gdeliveryd

gdeliveryd : $(SHAREOBJ) $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ)  $(LOGSTUB) $(LOGOBJ)
#	$(LD) $(LDFLAGS) -o $@ $(OUTEROBJS) $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) $(LOGSTUB) $(LOGOBJ)  -lpcre -lcrypto -ldl -lssl $(LIBICONV)
# 9.3 reverse
	$(LD) $(LDFLAGS) -o $@ $(LOGOBJ) $(LOGSTUB)  $(SHAREOBJ) $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ) -lpcre -lcrypto -ldl -lssl $(LIBICONV)

include ../mk/gcc.rules.mk


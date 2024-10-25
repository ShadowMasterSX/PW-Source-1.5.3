AR  = ar
CPP = g++ -g
CC  = g++
#CC = icpc -w -xN 
LD  = g++
#LD  = icpc -w -xN
#RM  = rm

INCLUDES = -I. -I$(TOP_SRCDIR) -I$(TOP_SRCDIR)/include -I$(TOP_SRCDIR)/xinclude -I$(TOP_SRCDIR)/rpc -I$(TOP_SRCDIR)/inl -I$(TOP_SRCDIR)/rpcdata

#DEFINES = -Wall -D_GNU_SOURCE -pthread -D__OLD_IOLIB_COMPATIBLE__ #-mcpu=pentium4
DEFINES = -Wall -D_GNU_SOURCE -pthread #-mcpu=pentium4

ifeq ($(DEBUG_VERSION),true)
	DEFINES += -D_DEBUGINFO -D_DEBUG -g -ggdb -O0 
else
	DEFINES += -g -O3
endif

LDFLAGS += -L$(TOP_SRCDIR)/xlib -lio32 -llog32 -lpthread

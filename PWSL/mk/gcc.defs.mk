AR := ar
CPP := g++ -g
CC := g++
# Alternative: CC := icpc -w -xN
LD := g++
# Alternative: LD := icpc -w -xN

INCLUDES := -I. -I$(TOP_SRCDIR) -I$(TOP_SRCDIR)/include -I$(TOP_SRCDIR)/xinclude -I$(TOP_SRCDIR)/rpc -I$(TOP_SRCDIR)/inl -I$(TOP_SRCDIR)/rpcdata

DEFINES := -Wall -D_GNU_SOURCE -pthread

ifeq ($(DEBUG_VERSION), true)
	DEFINES += -D_DEBUGINFO -D_DEBUG -g -ggdb -O0
else
	DEFINES += -g -O3
endif

LDFLAGS += -L$(TOP_SRCDIR)/xlib -lio32 -llog32 -lpthread

IOPATH=/mnt/c/Users/GUSTAVO/Desktop/pw152src_client-master/pw152src_client-master/server/iolib
BASEPATH=/mnt/c/Users/GUSTAVO/Desktop/pw152src_client-master/pw152src_client-master/server/cgame

INC=-I$(BASEPATH)/include -I$(BASEPATH) -I$(IOPATH)/inc -I$(BASEPATH)/libcommon
IOLIB_OBJ=$(BASEPATH)/libgs/gs/*.o $(BASEPATH)/libgs/io/*.o $(BASEPATH)/libgs/db/*.o $(BASEPATH)/libgs/log/*.o 
CMLIB=$(BASEPATH)/libcommon.a $(BASEPATH)/libonline.a $(IOLIB_OBJ) $(BASEPATH)/collision/libTrace.a
DEF = -DLINUX -D_DEBUG  -D__THREAD_SPIN_LOCK__ -DUSE_LOGCLIENT
#DEF += -D_CHECK_MEM_ALLOC
#DEF += -D__USE_ICPC__  
#DEF += -D__TEST_PERFORMANCE__
DEF += -D__USER__=\"$(USER)\"

THREAD = -D_REENTRANT -D_THREAD_SAFE 
THREADLIB = -pthread  
PCRELIB = -lpcre
LIBICONV =
ALLLIB = $(THREADLIB) $(PCRELIB) -lssl -lcrypto $(LIBICONV)  #/usr/lib/libcrypto.a
CFLAGS  = -std=gnu++11 -w -Wno-parentheses -fpermissive -Wno-write-strings -Wno-unused-local-typedefs -Wno-stringop-truncation -Wno-error=parentheses -Wno-class-memaccess #-Wall -pipe
CPPFLAGS = -std=gnu++11 -w -Wno-parentheses -fpermissive -Wno-write-strings -Wno-unused-local-typedefs -Wno-stringop-truncation -Wno-error=parentheses -Wno-class-memaccess #-Wall -pipe
OPTIMIZE = -O0
#-O2 -ipo
CC=gcc -m32  $(DEF) $(OPTIMIZE) $(THREAD) $(CFLAGS) -g -ggdb
CPP=g++ -m32 $(DEF) $(OPTIMIZE) $(THREAD) $(CPPFLAGS) -g -ggdb
#-pedantic
LD=g++ -m32 -g -std=gnu++11 -L/usr/local/ssl/lib $(OPTIMIZE) $(THREADLIB) 
AR=ar crs 
ARX=ar x

#
# include dependency files if they exist
#

ifneq ($(wildcard .depend),)
include .depend
endif

ifeq ($(TERM),cygwin)
THREADLIB = -lpthread
CMLIB += /usr/lib/libgmon.a
DEF += -D__CYGWIN__
endif

dep:
	$(CC) -MM $(INC)  -c *.c* > .depend


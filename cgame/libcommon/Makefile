ALLOBJ=allocator.o  base64.o  mtime.o  rwlock.o    strfunc.o  verbose.o astring.o    crc.o        parse.o    spinlock.o  strtok.o   verifyname.o csection.o interlocked.o cmnet.o timer.o conf.o threadpool.o amemory.o amemobj.o arandomgen.o ASSERT.o

LIBOBJ=allocator.o  base64.o  mtime.o  rwlock.o    strfunc.o  verbose.o astring.o    crc.o        parse.o    spinlock.o  strtok.o   verifyname.o csection.o interlocked.o cmnet.o timer.o conf.o threadpool.o amemory.o arandomgen.o ASSERT.o

all:	$(ALLOBJ)
	$(AR) ../libcommon.a $(LIBOBJ)

lib:	$(LIBOBJ)
	$(AR) ../libcommon.a $(LIBOBJ)

FORCE:

clean:	FORCE
	rm -f *.o; rm -f ../libcommon.a; rm -f .depend;

.cpp.o:
	$(CPP) -c $(INC) $< -o $@ 

include ../Rules.make

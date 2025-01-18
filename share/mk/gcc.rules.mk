# gcc.rules.mk
# Should be included last, but before any rules overrides

######## Common targets

ifeq ($(SINGLE_THREAD),true)
$(SHAREOBJ): %.o: %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
$(LOGOBJ): %.o: %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
$(LOGSTUB): %.o: %.cxx
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
else
$(SHAREOBJ): %_m.o: %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
$(LOGOBJ): %_m.o: %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
$(LOGSTUB): %_m.o: %.cxx
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
endif

.c.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

.cpp.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

.s.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

.cxx.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

%.o : %.cxx
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

# Rule for making executables
%: %.o
	$(LD) $(LDFLAGS) -rdynamic -o $@ $^ $(LDLIBS)

%.so:
	$(LD) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

%.a:
	ar rsv $@ $^

$(AUTOGENS): rpcalls.xml
	../rpc/rpcgen.pl rpcalls.xml

######## Clean targets

.PHONY: ftclean clean distclean

ftclean:
	rm -rf $(OBJS) $(EXES) $(CLEAN)

clean:
	rm -rf $(SHAREOBJ) $(OUTEROBJS) $(LIBOBJS) $(OBJS) $(EXES) $(CLEAN) $(DISTCLEAN) $(LOGOBJ) $(LOGSTUB)

distclean:
	rm -rf $(SHAREOBJ) $(OUTEROBJS) $(LIBOBJS) $(OBJS) $(EXES) $(CLEAN) $(DISTCLEAN)

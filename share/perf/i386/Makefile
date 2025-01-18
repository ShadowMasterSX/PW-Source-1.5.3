.PHONY: all clean

CC := g++ -m32 -w -std=gnu++17

OBJS := md5.o sha1.o rc4.o mppc256.o base64.o crc32.o aes.o bf.o

all: libperf.a

.s.o:
	$(CC) -c $<

libperf.a: $(OBJS)
	ar -ru $@ $(OBJS)

perf.lib: libperf.a
	objcopy --output-target=coff-i386 --prefix-symbols="_" --change-leading-char $< $@

clean:
	rm -f $(OBJS) libperf.a perf.lib

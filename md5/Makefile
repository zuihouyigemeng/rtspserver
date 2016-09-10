

CFLAGS +=-I ../include -I ../

LIBMD5 = md5c.a
OBJS  = md5c.o
LOGJS = md5c.lo
SOURCE = md5c.c

all:$(LIBMD5)

$(LIBMD5):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a *.la

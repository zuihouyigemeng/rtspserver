CFLAGS += -I ../include -I ../

LIMMULTICAST = multicast.a
OBJS  = is_valid_multicast_address.o

SOURCE = is_valid_multicast_address.c

all:$(LIMMULTICAST)

$(LIMMULTICAST):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 

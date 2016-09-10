CFLAGS += -I ../include -I ../

LIBEVENTLOOP = eventloop.a

OBJS  = add_client.o\
        eventloop.o\
        rtsp_server.o\
        schedule_connections.o

SOURCE = add_client.c\
        eventloop.c\
        rtsp_server.c\
        schedule_connections.c

all:$(LIBEVENTLOOP)

$(LIBEVENTLOOP):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	

.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o    *.a 

CFLAGS += -I ../include -I ../

LIMSOCKET = socket.a
OBJS  = get_address.o\
        sock_ntop_host.o\
        tcp_accept.o\
        tcp_close.o\
        tcp_connect.o\
        tcp_listen.o\
        tcp_read.o\
        tcp_write.o\
        udp_close.o\
        udp_connect.o\
        udp_open.o

SOURCE = get_address.c\
        sock_ntop_host.c\
        tcp_accept.c\
        tcp_close.c\
        tcp_connect.c\
        tcp_listen.c\
        tcp_read.c\
        tcp_write.c\
        udp_close.c\
        udp_connect.c\
        udp_open.c

all:$(LIMSOCKET)

$(LIMSOCKET):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o   *.a 

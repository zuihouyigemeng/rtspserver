CFLAGS += -I ../include -I ../

LIBRTP = rtp.a
OBJS  = RTP_get_port_pair.o\
        RTP_port_pool_init.o\
        RTP_recv.o\
        RTP_release_port_pair.o\
        RTP_send_packet.o\
        RTP_sendto.o\
        RTP_session_destroy.o

SOURCE = RTP_get_port_pair.c\
         RTP_port_pool_init.c\
         RTP_recv.c\
         RTP_release_port_pair.c\
         RTP_send_packet.c\
         RTP_sendto.c\
         RTP_session_destroy.c

all:$(LIBRTP)

$(LIBRTP):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
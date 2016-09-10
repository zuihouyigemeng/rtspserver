CFLAGS += -I ../include -I ../

LIBRTCP = rtcp.a
OBJS  = RTCP_get_RR_received.o\
        RTCP_get_fract_lost.o\
        RTCP_get_jitter.o\
        RTCP_get_pkt_lost.o\
        RTCP_get_total_packet.o\
        RTCP_get_total_payload_octet.o\
        RTCP_handler.o\
        RTCP_recv_packet.o\
        RTCP_send_packet.o

SOURCE = RTCP_get_RR_received.c\
         RTCP_get_fract_lost.c\
         RTCP_get_jitter.c\
         RTCP_get_pkt_lost.c\
         RTCP_get_total_packet.c\
         RTCP_get_total_payload_octet.c\
         RTCP_handler.c\
         RTCP_recv_packet.c\
         RTCP_send_packet.c

all:$(LIBRTCP)

$(LIBRTCP):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
CFLAGS += -I ../include -I ../

LIBRTSP = rtsp.a
OBJS  =  RTSP_describe.o\
         RTSP_discard_msg.o\
         RTSP_full_msg_rcvd.o\
         RTSP_handler.o\
         RTSP_initserver.o\
         RTSP_options.o\
         RTSP_pause.o\
         RTSP_play.o\
         RTSP_remove_msg.o\
         RTSP_setup.o\
         RTSP_state_machine.o\
         RTSP_teardown.o\
         RTSP_valid_response_msg.o\
         RTSP_validate_method.o\
         add_time_stamp.o\
         bwrite.o\
         get_stat.o\
         max_connection.o\
         parse_url.o\
         send_describe_reply.o\
         send_options_reply.o\
         send_pause_reply.o\
         send_play_reply.o\
         send_redirect_3xx.o\
         send_reply.o\
         send_setup_reply.o\
         send_teardown_reply.o

SOURCE = RTSP_describe.c\
         RTSP_discard_msg.c\
         RTSP_full_msg_rcvd.c\
         RTSP_handler.c\
         RTSP_initserver.c\
         RTSP_options.c\
         RTSP_pause.c\
         RTSP_play.c\
         RTSP_remove_msg.c\
         RTSP_setup.c\
         RTSP_state_machine.c\
         RTSP_teardown.c\
         RTSP_valid_response_msg.c\
         RTSP_validate_method.c\
         add_time_stamp.c\
         bwrite.c\
         get_stat.c\
         max_connection.c\
         parse_url.c\
         send_describe_reply.c\
         send_options_reply.c\
         send_pause_reply.c\
         send_play_reply.c\
         send_redirect_3xx.c\
         send_reply.c\
         send_setup_reply.c\
         send_teardown_reply.c
         
all:$(LIBRTSP)

$(LIBRTSP):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
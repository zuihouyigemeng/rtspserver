CFLAGS += -I ../include -I ../

LIBSDP = sdp.a
OBJS  =  get_SDP_descr.o\
         get_SDP_session_id.o\
         get_SDP_user_name.o\
         get_SDP_version.o\
         get_media_descr.o

SOURCE = get_SDP_descr.c\
         get_SDP_session_id.c\
         get_SDP_user_name.c\
         get_SDP_version.c\
         get_media_descr.c

         
all:$(LIBSDP)

$(LIBSDP):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
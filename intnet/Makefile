CFLAGS += -I ../include -I ../

LIBINTNT = intnet.a
OBJS  = change_check.o\
         downgrade_GSM.o\
         downgrade_L16.o\
         downgrade_MP3.o\
         half_GSM.o\
         half_L16.o\
         half_MP3.o\
         priority_decrease.o\
         priority_increase.o\
         stream_change.o\
         upgrade_GSM.o\
         upgrade_MP3.o

SOURCE = change_check.c\
         downgrade_GSM.c\
         downgrade_L16.c\
         downgrade_MP3.c\
         half_GSM.c\
         half_L16.c\
         half_MP3.c\
         priority_decrease.c\
         priority_increase.c\
         stream_change.c\
         upgrade_GSM.c\
         upgrade_MP3.c
         
all:$(LIBINTNT)

$(LIBINTNT):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
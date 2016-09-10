CFLAGS += -I ../include -I ../

LIBSCHEDULE = schedule.a
OBJS  =  schedule_add.o\
         schedule_do.o\
         schedule_init.o\
         schedule_remove.o\
         schedule_resume.o\
         schedule_start.o\
         schedule_stop.o

SOURCE = schedule_add.c\
         schedule_do.c\
         schedule_init.c\
         schedule_remove.c\
         schedule_resume.c\
         schedule_start.c\
         schedule_stop.c
         
all:$(LIBSCHEDULE)

$(LIBSCHEDULE):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
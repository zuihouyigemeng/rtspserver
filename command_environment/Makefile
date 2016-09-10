CFLAGS += -I ../include -I ../

LIBCMDENV = cmd_env.a
OBJS  = command_environment.o
SOURCE = command_environment.c

all:$(LIBCMDENV)

$(LIBCMDENV):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
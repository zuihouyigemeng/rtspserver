#CROSS = arm-uclinux-elf-

#CPP = $(CROSS)g++
#CC  = $(CROSS)gcc
#LD  = $(CROSS)ld
#AR  = $(CROSS)ar

#CFLAGS = -D_REENTRANT -DLINUX -Wall -O2 -I ../include -I ../
#LFLAGS = -Wl,-elf2flt
#LDFLAGS = -Wl,-elf2flt
#AFLAGS	+= -rcu

CFLAGS += -I ../include -I ../  

LIMBUFFERPOOL = bufferpool.a
OBJS  = OMSbuff_addpage.o\
        OMSbuff_dump.o\
        OMSbuff_free.o\
        OMSbuff_getreader.o\
        OMSbuff_getslot.o\
        OMSbuff_gotreader.o\
        OMSbuff_isempty.o\
        OMSbuff_new.o\
        OMSbuff_nextts.o\
        OMSbuff_read.o\
        OMSbuff_ref.o\
        OMSbuff_shm_addpage.o\
        OMSbuff_shm_create.o\
        OMSbuff_shm_destroy.o\
        OMSbuff_shm_map.o\
        OMSbuff_shm_remap.o\
        OMSbuff_shm_unmap.o\
        OMSbuff_sync.o\
        OMSbuff_unref.o\
        OMSbuff_write.o\
        fnc_ipc_name.o \

SOURCE = OMSbuff_addpage.c\
         OMSbuff_dump.c\
         OMSbuff_free.c\
         OMSbuff_getreader.c\
         OMSbuff_getslot.c\
         OMSbuff_gotreader.c\
         OMSbuff_isempty.c\
         OMSbuff_new.c\
         OMSbuff_nextts.c\
         OMSbuff_read.c\
         OMSbuff_ref.c\
         OMSbuff_shm_addpage.c\
         OMSbuff_shm_create.c\
         OMSbuff_shm_destroy.c\
         OMSbuff_shm_map.c\
         OMSbuff_shm_remap.c\
         OMSbuff_shm_unmap.c\
         OMSbuff_sync.c\
         OMSbuff_unref.c\
         OMSbuff_write.c\
         fnc_ipc_name.c \

all:$(LIMBUFFERPOOL)

$(LIMBUFFERPOOL):$(OBJS)
	$(AR) $(AFLAGS) $@ $^ 
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 

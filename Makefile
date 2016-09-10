CROSS = arm-uclinux-elf-

CPP = $(CROSS)g++
CC  = $(CROSS)gcc
LD  = $(CROSS)ld
AR  = $(CROSS)ar

CFLAGS = -D_REENTRANT -DLINUX -Wall -O2 -mcpu=arm940t -fomit-frame-pointer -fno-builtin -Os -DRTSP_DEBUG -DRTSP_METHOD_LOG
LFLAGS = -Wl,-elf2flt
LDFLAGS = -Wl,-elf2flt
AFLAGS	+= -rcu
export CROSS CPP CC LD AR CFLAGS LFLAGS LDFLAGS AFLAGS
DIST = /cygdrive/d/code_base/ch/alcatel
EXEC := $(DIST)/rtsp_server

MAINOBJS = src/main.o src/fncheader.o

SUBLIBS = eventloop/eventloop.a\
          rtsp/rtsp.a\
          sdp/sdp.a\
          schedule/schedule.a\
          intnet/intnet.a\
          rtcp/rtcp.a\
          rtp/rtp.a\
          mediainfo/mediainfo.a\
          bufferpool/bufferpool.a\
          multicast/multicast.a\
          socket/socket.a\
          command_environment/cmd_env.a\
          src/prefs.a\
          src/utils.a\
          src/log.a\
          md5/md5c.a
 
 SUBDIRS =  bufferpool\
          socket\
          md5\
          multicast\
          command_environment\
          eventloop\
          rtp\
          rtcp\
          intnet\
          rtsp\
          mediainfo\
          sdp\
          schedule\
          src    

         
all:standardall $(EXEC) 

standardall:subdirs
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do \
			echo "making all in `pwd`/$$i"; \
			( cd $$i ; $(MAKE) ) ; \
			if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
		done \
	fi
#
#$(EXEC):
#	$(LINK) $(CFLAGS)  -o $@ $(MAINOBJS)  $(SUBLIBS) -lm -lpthread

$(EXEC): $(MAINOBJS)  $(SUBLIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS) -v -lm -lpthread;

clean:cleansubdirs
	rm -f *.o *.lo   *.a  $(EXEC)* 
	
cleansubdirs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do \
			echo "making clean in `pwd`/$$i"; \
			( cd $$i ; $(MAKE) clean) ;   \
			if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
		done \
	fi	
	
# These aren't real targets, let gnu's make know that.
.PHONY: clean cleansubdirs \
	all subdirs standardall 

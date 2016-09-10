CFLAGS += -I ../include -I ../

LIBMEDIAINFO = mediainfo.a
OBJS  = calculate_skip.o\
         common_utils.o\
         default_selection_criterion.o\
         enum_media.o\
         free_GSM.o\
         free_H26L.o\
         free_L16.o\
         free_MP2T.o\
         free_MP4ES.o\
         free_MPA.o\
         free_MPV.o\
         get_frame.o\
         get_media_entry.o\
         load_GSM.o\
         load_H26L.o\
         load_L16.o\
         load_MP2T.o\
         load_MP4ES.o\
         load_MPA.o\
         load_MPV.o\
         media_is_empty.o\
         mediaclose.o\
         mediacpy.o\
         mediaopen.o\
         msec2tick.o\
         parse_SD_file.o\
         read_GSM.o\
         read_H26L.o\
         read_MP3.o\
         read_MPEG4ES_utils.o\
         read_MPEG4ES_video.o\
         read_MPEGSYSTEM_utils.o\
         read_MPEG_system.o\
         read_MPEG_ts.o\
         read_MPEG_utils.o\
         read_MPEG_video.o\
         read_PCM.o\
         read_dim.o\
         register_media.o\
         rtp_shm.o\
         search_media.o\
         tick2msec.o\
         validate_stream.o

SOURCE = calculate_skip.c\
         common_utils.c\
         default_selection_criterion.c\
         enum_media.c\
         free_GSM.c\
         free_H26L.c\
         free_L16.c\
         free_MP2T.c\
         free_MP4ES.c\
         free_MPA.c\
         free_MPV.c\
         get_frame.c\
         get_media_entry.c\
         load_GSM.c\
         load_H26L.c\
         load_L16.c\
         load_MP2T.c\
         load_MP4ES.c\
         load_MPA.c\
         load_MPV.c\
         media_is_empty.c\
         mediaclose.c\
         mediacpy.c\
         mediaopen.c\
         msec2tick.c\
         parse_SD_file.c\
         read_GSM.c\
         read_H26L.c\
         read_MP3.c\
         read_MPEG4ES_utils.c\
         read_MPEG4ES_video.c\
         read_MPEGSYSTEM_utils.c\
         read_MPEG_system.c\
         read_MPEG_ts.c\
         read_MPEG_utils.c\
         read_MPEG_video.c\
         read_PCM.c\
         read_dim.c\
         register_media.c\
         rtp_shm.c\
         search_media.c\
         tick2msec.c\
         validate_stream.c
         
all:$(LIBMEDIAINFO)

$(LIBMEDIAINFO):$(OBJS)
	$(AR) $(AFLAGS) $@ $^
	
.c.o:
	$(CC) -c $(CFLAGS) $<	-o $@
clean:
	rm -f *.o *.lo   *.a 
/* * 
 *  $Id: bufferpool.h 327 2006-03-28 16:14:41Z shawill $
 *  
 *  This file is part of Fenice
 *
 *  Fenice -- Open Media Server
 *
 *  Copyright (C) 2004 by
 * 
 *- Giampaolo Mancini <giampaolo.mancini@polito.it>
 *- Francesco Varano  <francesco.varano@polito.it>
 *- Marco Penno           <marco.penno@polito.it>
 *- Federico Ridolfo      <federico.ridolfo@polito.it>
 *- Eugenio Menegatti     <m.eu@libero.it>
 *- Stefano Cau
 *- Giuliano Emma
 *- Stefano Oldrini
 * 
 *  Fenice is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Fenice is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Fenice; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * */
 
 /*
 * Fenice's bufferpool is projected to support ONE AND ONLY ONE producer and many consumers.
 * Each consumer have it's own OMSConsumer structure that is NOT SHARED with others readers.
 * So the only struct that must be locked with mutexes is OMSBuffer.
 * */

#ifndef _BUFFERPOOLH
#define _BUFFERPOOLH

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fenice/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

#define omsbuff_min(x,y) ((x) < (y) ? (x) : (y))
#define omsbuff_max(x,y) ((x) > (y) ? (x) : (y))

#define OMSBUFF_MEM_PAGE 9

#define OMSSLOT_DATASIZE 65000

#define OMSBUFF_SHM_CTRLNAME "Buffer"
#define OMSBUFF_SHM_SLOTSNAME "Slots"
#define OMSBUFF_SHM_PAGE OMSBUFF_MEM_PAGE        /*共享内存的页数*/   


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define OMSbuff_lock(buffer) pthread_mutex_lock(&buffer->control->syn)
#define OMSbuff_unlock(buffer) pthread_mutex_unlock(&buffer->control->syn)

typedef ptrdiff_t OMSSlotPtr;            /*long 类型变量*/                       

#define OMSSLOT_COMMON  uint16 refs; \
            uint64 slot_seq; /* monotone identifier of slot (NOT RTP seq) */ \
            double timestamp; \
            uint8 data[OMSSLOT_DATASIZE]; \
            uint32 data_size; \
            uint8 marker; \
            ptrdiff_t next;

typedef struct _OMSslot 
{
    OMSSLOT_COMMON
} OMSSlot;

typedef struct _OMSControl 
{
    uint16 refs;  /*共享缓存的消费者*/
    uint32 nslots;
    OMSSlotPtr write_pos; /*最后一次写操作的位置*/
    OMSSlotPtr valid_read_pos; /*新消费者的合法读位置*/
    pthread_mutex_t syn;        /*信号量*/
} OMSControl;

typedef enum {buff_local=0, buff_shm} OMSBufferType;

/*
 * 缓冲区结构体
 * 在通用的内存分配操作和共享内存操作下，指针有不同的含义
 * 使用共享内存的情况下，缓冲指针是相对共享内存开头的偏移量
 * 在此情况下，两个进程可以通过将mmap的返回值加入到地址中获得绝对地址
 * */
typedef struct _OMSbuffer {
    OMSBufferType type;                /*是否是共享内存*/
    OMSControl *control;             
    OMSSlot *slots;                        /*! Buffer head*/
    uint32 known_slots; /*最后一个已知的slot个数，仅仅在共享内存缓冲区情况有用 */
    char filename[PATH_MAX];
} OMSBuffer;

typedef struct _OMSconsumer {
    OMSSlotPtr read_pos; /*! read position*/
    OMSSlotPtr last_read_pos; /*! last read position . used for managing the slot addition*/
    uint64 last_seq;
    OMSBuffer *buffer;
    int32 frames;
    double firstts;
} OMSConsumer;

/*! This structure is usefull if you need to do some syncronization among different correlated buffers.
 * */
typedef struct _OMSAggregate {
    OMSBuffer *buffer;
    struct _OMSAggregate *next;
} OMSAggregate;

/*API函数定义，声明*/
OMSBuffer *OMSbuff_new(uint32);
OMSConsumer *OMSbuff_ref(OMSBuffer *);
void OMSbuff_unref(OMSConsumer *);
int32 OMSbuff_read(OMSConsumer *, uint32 *, uint8 *, uint8 *, uint32 *);
OMSSlot *OMSbuff_getreader(OMSConsumer *);
int32 OMSbuff_gotreader(OMSConsumer *);
int32 OMSbuff_write(OMSBuffer *, uint64, uint32, uint8, uint8 *, uint32);
OMSSlot *OMSbuff_getslot(OMSBuffer *);
OMSSlot *OMSbuff_addpage(OMSBuffer *, OMSSlot *);
//OMSSlot *OMSbuff_slotadd(OMSBuffer *, OMSSlot *);
void OMSbuff_free(OMSBuffer *);

int OMSbuff_isempty(OMSConsumer *);
double OMSbuff_nextts(OMSConsumer *);

/*Bufferpool 中的共享内存操作函数*/
OMSBuffer *OMSbuff_shm_create(char *, uint32);
OMSBuffer *OMSbuff_shm_map(char *);
OMSSlot *OMSbuff_shm_addpage(OMSBuffer *);
int OMSbuff_shm_remap(OMSBuffer *);

#define OMSbuff_shm_refresh(oms_buffer)	    \
(((oms_buffer->type == buff_shm) && (oms_buffer->known_slots != oms_buffer->control->nslots)) ? \
OMSbuff_shm_remap(oms_buffer) : 0)


int OMSbuff_shm_unmap(OMSBuffer *);
int OMSbuff_shm_destroy(OMSBuffer *);

/*IPC  名字*/
char *fnc_ipc_name(const char *, const char *);

// OMSSlotPtr manipulation
#define OMStoSlot(b, p) ((p<0) ? NULL : (&b->slots[p])) // used when a pointer could be NULL, otherwise we'll use buffe->slots[index]
#define OMStoSlotPtr(b, p) (p ? p - b->slots : -1)

/*消费者集合之间的同步*/
OMSAggregate *OMBbuff_aggregate(OMSAggregate *, OMSBuffer *);
int OMSbuff_sync(OMSAggregate *);


#ifdef ENABLE_DUMPBUFF
void OMSbuff_dump(OMSConsumer *, OMSBuffer *);
#else
#define OMSbuff_dump(x, y)
#endif


#endif


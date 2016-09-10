/* * 
 *  $Id: OMSbuff_shm_create.c 327 2006-03-28 16:14:41Z shawill $
 *  
 *  This file is part of Fenice
 *
 *  Fenice -- Open Media Server
 *
 *  Copyright (C) 2004 by
 *  	
 *	- Giampaolo Mancini	<giampaolo.mancini@polito.it>
 *	- Francesco Varano	<francesco.varano@polito.it>
 *	- Marco Penno		<marco.penno@polito.it>
 *	- Federico Ridolfo	<federico.ridolfo@polito.it>
 *	- Eugenio Menegatti 	<m.eu@libero.it>
 *	- Stefano Cau
 *	- Giuliano Emma
 *	- Stefano Oldrini
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
 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/mman.h>

#include <fenice/bufferpool.h>
#include <fenice/fnc_log.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*该函数在文件系统中创建共享内存对象，以便不同、独立的进程来访问它
 *注意:该函数并没有被fennice直接使用，但是这里还是给它实现
 *fennice仅仅是共享内存对象的使用者,生产者可能是其它的程序
 *Fenice is just the "consumer" of a shared memory object, while the producer is some other object like felix.
 * */

OMSBuffer *OMSbuff_shm_create (char *shm_name, uint32 buffer_size)
{
    /*定义局部变量*/
    OMSBuffer *buffer;
    OMSSlot *slots;
    OMSControl *control;
    int fd, shm_open_errno;
    uint32 index;
    char *shm_file_name; 
    pthread_mutexattr_t mutex_attr;

    if (!buffer_size) return NULL;

#if 0   /*yanf*/
    /* control struct shared memory object */
    if ( !(shm_file_name = fnc_ipc_name(shm_name, OMSBUFF_SHM_CTRLNAME)) ) return NULL;

    fd = shm_open(shm_file_name, O_RDWR | O_CREAT | O_EXCL, FILE_MODE);           /*打开共享内存文件*/
    shm_open_errno = errno;
#endif
    strcpy(shm_file_name,"../shm");
    fd = open(shm_file_name,O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
    shm_open_errno = errno;
    /*处理错误的情况，写入到日志中*/
    if ( (fd < 0) )
    {
        switch (shm_open_errno)
        {
            
            case EEXIST:
                fnc_log(FNC_LOG_ERR, "SHM object \"%s\" already exists! Perhaps some other apps are using it\n", shm_file_name);
                fnc_log(FNC_LOG_ERR, "TIP: If you are sure none is using it try deleting it manually.\n");
                break;
                
            case EINVAL:
                fnc_log(FNC_LOG_ERR, "Invalid name (%s) was given for shared memory object\n", shm_file_name);
                break;
                
            case EACCES:
                fnc_log(FNC_LOG_ERR, "Permission denied for shared memory object\n", shm_file_name);
                break;
                
            case ENOENT:
                fnc_log(FNC_LOG_ERR, "Could not create\n", shm_file_name);
                break;
                
            default:
                break;
                
        }
        
        fnc_log(FNC_LOG_ERR, "Could not open/create POSIX shared memory %s (OMSControl)\n", shm_file_name);
        free(shm_file_name);
        return NULL;
    }
    free(shm_file_name);  /*释放变量，其空间已经在fnc_ipc_name中被分配*/

    /*将文件裁减到指定的大小
     *如果文件的长度超长则被截断，否则被扩展，扩展的部分都被置0 
     */
    if (ftruncate(fd, sizeof(OMSControl)))                     
    {
        fnc_log(FNC_LOG_ERR, "Could not set correct size for shared memory object (OMSControl)\n");
        close(fd);
        return NULL;
    }

    /*将共享内存文件中的内容映射到共享内存,返回映射区内存地址
     * 关闭之后，共享内存的操作不会收到影响
    */
    control = mmap(NULL, sizeof(OMSControl), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    
    if ( control == MAP_FAILED ) 
    {
        fnc_log(FNC_LOG_FATAL, "SHM: error in mmap\n");
        return NULL;
    }

    /*初始化OMS*/ 
    if ( pthread_mutexattr_init(&mutex_attr) || pthread_mutex_init(&control->syn, &mutex_attr) )
    {
        munmap(control, sizeof(OMSControl));
        return NULL;
    }

    /*信号量锁*/
    pthread_mutex_lock(&control->syn);
    control->refs = 0;
    control->nslots = buffer_size;
 
#if 0    /*yanf*/
    /*** slots mapping in shared memory ***/
    if ( !(shm_file_name = fnc_ipc_name(shm_name, OMSBUFF_SHM_SLOTSNAME)) ) return NULL;

    fd = shm_open(shm_file_name, O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
    shm_open_errno = errno;
    free(shm_file_name);
#endif
    strcpy(shm_file_name,"../shm");
    fd = open(shm_file_name,O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
    shm_open_errno = errno;
    if ( (fd < 0) ) 
    {
        switch (shm_open_errno) 
        {
            case EEXIST:
                fnc_log(FNC_LOG_ERR, "SHM object \"%s\" already exists! Perhaps some other apps are using it\n", shm_file_name);
                fnc_log(FNC_LOG_ERR, "TIP: If you are sure none is using it try deleting it manually.\n");
                break;
            
            case EINVAL:
                fnc_log(FNC_LOG_ERR, "Invalid name (%s) was given for shared memory object\n", shm_file_name);
                break;
                
            default:
                break;
        }
        fnc_log(FNC_LOG_ERR, "Could not open/create POSIX shared memory (OMSSlots)\n");
        munmap(control, sizeof(OMSControl));
        return NULL;
    }

    /*改变共享内存对象的长度*/
    if (ftruncate(fd, buffer_size * sizeof(OMSSlot))) 
    {
        fnc_log(FNC_LOG_ERR, "Could not set correct size for shared memory object (OMSControl)\n");
        close(fd);
        munmap(control, sizeof(OMSControl));
        return NULL;
    }

    /*映射*/
    slots = mmap(NULL, buffer_size * sizeof(OMSSlot), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    
    if ( slots == MAP_FAILED ) 
    {
        fnc_log(FNC_LOG_FATAL, "SHM: error in mmap\n");
        munmap(control, sizeof(OMSControl));
        return NULL;
    }

    /*初始化OMSSlot，所有的slots循环连接*/ 
    for(index=0; index<buffer_size-1; index++) 
    {
        slots[index].refs = 0;
        slots[index].slot_seq = 0;
        slots[index].next = index + 1; 
    }

    slots[index].next = 0; 
    slots[index].slot_seq = 0;
    slots[index].refs = 0; 

    
    /*设置写slots的位置*/
    control->write_pos = buffer_size-1;
    control->valid_read_pos = 0; 

    if ( !(buffer = (OMSBuffer *)malloc(sizeof(OMSBuffer))) )
    {
        munmap(slots, control->nslots*sizeof(OMSSlot));
        munmap(control, sizeof(OMSControl));
        return NULL;
    }

    /*填充构造的buffer的变量*/
    buffer->type = buff_shm;
    buffer->slots = slots;
    buffer->known_slots = control->nslots;
    strncpy(buffer->filename, shm_name, sizeof(buffer->filename)-1);

    buffer->slots = slots;
    buffer->control = control;

    pthread_mutex_unlock(&control->syn);

    return buffer;
}

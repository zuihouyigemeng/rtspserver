/* * 
 *  $Id: main.c 176 2005-06-28 14:47:59Z shawill $
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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <config.h>
#ifdef WIN32
#include <winsock2.h>
#endif
#include <fenice/socket.h>
#include <fenice/eventloop.h>
#include <fenice/prefs.h>
#include <fenice/schedule.h>
#include <fenice/utils.h>
#include <fenice/command_environment.h>
#include <fenice/fnc_log.h>
/*#include <sys/types.h>*/ /*fork*/
/*#include <unistd.h>  */  /*fork*/

inline void fncheader(void);      /* defined in src/fncheader.c */

int main(int argc, char **argv)
{
   trace_point();
   tsocket main_fd;
    unsigned int port;
    struct timespec ts = { 0, 0 };

    fncheader();

    /*解析并分离命令行参数，返回错误的个数 */
/*
    if(0);//(command_environment(argc, argv))
    {
        trace_point();
        return 0;
    }
*/
	/* 读取静态变量，获取端口号*/
    port = prefs_get_port();

#ifdef WIN32
{
    int err;
    unsigned short wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) 
    {
        fnc_log(FNC_LOG_ERR,"Could not detect Windows socket support.\n");
        fnc_log(FNC_LOG_ERR,"Make sure WINSOCK.DLL is installed.\n");
        return 1;
    }
}
#endif

    printf("CTRL-C terminate the server.\n");
    fnc_log(FNC_LOG_INFO,"Waiting for RTSP connections on port %d...\n", port);
    
    main_fd = tcp_listen(port);

    /* 初始化schedule_list 队列，创建调度线程，参考 schedule.c */
    if (schedule_init() == ERR_FATAL) 
    {
        fnc_log(FNC_LOG_FATAL,"Fatal: Can't start scheduler. Server is aborting.\n");
        return 0;
    }

    /* 将所有可用的RTP端口号放入到port_pool[MAX_SESSION] 中 */
    RTP_port_pool_init(RTP_DEFAULT_PORT);

    while (1) 
    {
        nanosleep(&ts, NULL);

        /*查找收到的rtsp连接，
          * 对每一个连接产生所有的信息放入到结构体rtsp_list中
          */
        trace_point();
        eventloop(main_fd);
    }

#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}

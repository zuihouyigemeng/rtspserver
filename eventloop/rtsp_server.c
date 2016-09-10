/* * 
 *  $Id: rtsp_server.c 351 2006-06-01 17:58:07Z shawill $
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
#include <string.h>

#include <fenice/eventloop.h>
#include <fenice/utils.h>
#include <fenice/rtsp.h>
#include <fenice/fnc_log.h>

#include "../config.h"

int rtsp_server(RTSP_buffer *rtsp)
{
    /*DEBUG_PRINTF("entering rtsp_server!\n");*/  
    
    fd_set rset,wset;       /*读写I/O描述集*/
    struct timeval t;
    int size;
    char buffer[RTSP_BUFFERSIZE+1]; /* +1 to control the final '\0'*/
    int n;
    int res;
    RTSP_session *q=NULL;
    RTP_session *p=NULL;

    if (rtsp == NULL)
    {
        return ERR_NOERROR;
    }

    /*变量初始化*/
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    t.tv_sec=0;                             /*select 时间间隔*/
    t.tv_usec=100000;

    FD_SET(rtsp->fd,&rset);  
    if (rtsp->out_size>0)
    {
        FD_SET(rtsp->fd,&wset); 
    }

    /*调用select等待对应描述符变化*/
    if (select(MAX_FDS,&rset,&wset,0,&t)<0) 
    {
        fnc_log(FNC_LOG_ERR,"select error\n");
        send_reply(500, NULL, rtsp);
        return ERR_GENERIC; //errore interno al server
    }

    
    /*有可供读进的rtsp包*/
    if (FD_ISSET(rtsp->fd,&rset)) 
    {
        memset(buffer,0,sizeof(buffer));
        size=sizeof(buffer)-1;  /*最后一位用于填充字符串结束标识*/

        /*读入数据到缓冲区中*/
        n= tcp_read(rtsp->fd,buffer,size);
        if (n==0) 
        {
            return ERR_CONNECTION_CLOSE;
        }

        if (n<0) 
        {
            fnc_log(FNC_LOG_DEBUG,"read() error in rtsp_server()\n");
            send_reply(500, NULL, rtsp);                                                        /*服务器内部错误消息*/
            return ERR_GENERIC;
        }

        /*检查读入的数据是否产生溢出*/
        if (rtsp->in_size+n>RTSP_BUFFERSIZE) 
        {
            fnc_log(FNC_LOG_DEBUG,"RTSP buffer overflow (input RTSP message is most likely invalid).\n");
            send_reply(500, NULL, rtsp);
            return ERR_GENERIC;/*数据溢出错误*/
        }

        fnc_log(FNC_LOG_VERBOSE,"INPUT_BUFFER was:\n");
#ifdef VERBOSE
        dump_buffer(buffer);
#endif

        /*填充数据*/
        memcpy(&(rtsp->in_buffer[rtsp->in_size]),buffer,n);
        rtsp->in_size+=n;

        /*处理缓冲区的数据，进行rtsp处理*/
        if ((res=RTSP_handler(rtsp))==ERR_GENERIC) 
        {
            fnc_log(FNC_LOG_ERR,"Invalid input message.\n");
            return ERR_NOERROR;
        }
    }

    /*有发送数据*/
    if (FD_ISSET(rtsp->fd,&wset)) 
    {
        if (rtsp->out_size>0) 
        {
            /*将需要发送的数据写入到缓冲区中*/
            n= tcp_write(rtsp->fd,rtsp->out_buffer,rtsp->out_size);
            if (n<0) 
            {
                fnc_log(FNC_LOG_ERR,"tcp_write() error in rtsp_server()\n"); 
                send_reply(500, NULL, rtsp);
                return ERR_GENERIC; //errore interno al server
            }
            rtsp->out_size-=n;
            fnc_log(FNC_LOG_VERBOSE,"OUTPUT_BUFFER was:\n");
#ifdef VERBOSE
            dump_buffer(rtsp->out_buffer);
#endif
        }
    }
#ifdef POLLED
    schedule_do(0);                    /*如果设置了轮询，就可以在此处重新执行一下*/
#endif

    /*rtcp控制处理,检查读入RTCP数据报*/
    if ( (q=rtsp->session_list) == NULL )
        return ERR_NOERROR;

    for (p=q->rtp_session; p!=NULL; p=p->next) 
    {

        if (!p->started)
        {
            q->cur_state = READY_STATE; /*设置状态为就绪态*/
            /*free della struttura rtp TODO*/	
        }
        else   
        {   
            /*当前状态为初始态*/
            if (p->transport.rtcp_fd_in >= 0) 
            {
                    FD_ZERO(&rset);
                    t.tv_sec=0;
                    t.tv_usec=100000;
                    FD_SET(p->transport.rtcp_fd_in,&rset);

                    if (select(MAX_FDS,&rset,0,0,&t)<0) 
                    {
                        fnc_log(FNC_LOG_ERR,"select error\n");
                        send_reply(500, NULL, rtsp);
                        return ERR_GENERIC;   /*服务器内部错误*/
                    }
                    
                    if (FD_ISSET(p->transport.rtcp_fd_in,&rset)) 
                    {       
                        /* 有需要读入的rtcp包*/
                        if (RTP_recv(p, rtcp_proto)<0) 
                        {     
                            fnc_log(FNC_LOG_VERBOSE,"Input RTCP packet Lost\n");
                        }
                        else 
                        {
                            RTCP_recv_packet(p);  /*填充缓冲区内容*/
                        }
                        fnc_log(FNC_LOG_VERBOSE,"IN RTCP\n");
                    }
                    else
                    {
                    }
            }   
            else
            {
            }
  
        }
    
	}
	/*DEBUG_PRINTF("leaving rtsp_server!no error occur!\n");*/                   /*just for debug*/
    return ERR_NOERROR;
}

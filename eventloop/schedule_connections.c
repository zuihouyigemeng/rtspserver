/* * 
 *  $Id: schedule_connections.c 351 2006-06-01 17:58:07Z shawill $
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
#include <stdlib.h>
#include <unistd.h>

#include <fenice/socket.h>
#include <fenice/eventloop.h>
#include <fenice/utils.h>
#include <fenice/rtsp.h>
#include <fenice/fnc_log.h>
#include <fenice/schedule.h>
#include <fenice/bufferpool.h>

#include "../config.h"

int stop_schedule = 0;
extern int num_conn;

void schedule_connections(RTSP_buffer **rtsp_list, int *conn_count)
{
    int res;
    RTSP_buffer *p=*rtsp_list,*pp=NULL;
    RTP_session *r=NULL, *t=NULL;

    while (p!=NULL)
    {
        if ((res = rtsp_server(p))!=ERR_NOERROR) 
        {
            if (res==ERR_CONNECTION_CLOSE || res==ERR_GENERIC)
            {
                /*连接已经关闭*/
                if (res==ERR_CONNECTION_CLOSE)
                    fnc_log(FNC_LOG_INFO,"RTSP connection closed by client.\n");
                else
                    fnc_log(FNC_LOG_INFO,"RTSP connection closed by server.\n");

                /*客户端在发送TEARDOWN 之前就截断了连接，但是会话却没有被释放*/
                if (p->session_list!=NULL) 
                { 
                    r=p->session_list->rtp_session;
                    /*释放所有会话*/
                    while (r!=NULL)
                    {
                        t = r->next;
                        schedule_remove(r->sched_id);
                        r=t;
                    }
                    /*释放链表头指针*/
                    free(p->session_list);
                    p->session_list=NULL;
                    fnc_log(FNC_LOG_WARN,"WARNING! RTSP connection truncated before ending operations.\n");
                }
                
                // wait for 
                close(p->fd);
                --*conn_count;
                num_conn--;

                /*释放rtsp缓冲区*/
                if (p==*rtsp_list) 
                {
                    *rtsp_list=p->next;
                    free(p);
                    p=*rtsp_list;
                } 
                else 
                {
                    pp->next=p->next;                
                    free(p);
                    p=pp->next;
                }

                /*适当情况下，释放调度器本身*/
                if (p==NULL && *conn_count<0) 
                {
                    fnc_log(FNC_LOG_DEBUG,"Fermo il thread\n");
                    stop_schedule=1;
                }
            }
            else
            {
                p=p->next;
            }
        } 
        else 
        {
            pp=p;
            p=p->next;
        }
    }

    /*DEBUG_PRINTF("~~~~~~~~~~~~~~~~~~end of schedule connections!\n");*/    /*just for debug,yanf*/
}

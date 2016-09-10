/* * 
 *  $Id: send_describe_reply.c 139 2005-05-12 14:55:17Z federico $
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

#include <config.h>
#include <fenice/rtsp.h>
#include <fenice/utils.h>
#include <fenice/fnc_log.h>

int send_describe_reply(RTSP_buffer * rtsp, char *object, description_format descr_format, char *descr)
{
    char *r;            /* 用于获取响应缓冲指针*/
    char *mb;        /* 消息头缓冲指针*/
    int mb_len;


    /* 分配空间，处理内部错误*/
    mb_len = 2048;
    mb = malloc(mb_len);
    r = malloc(mb_len + 1512);
    if (!r || !mb) 
    {
        fnc_log(FNC_LOG_ERR,"send_describe_reply(): unable to allocate memory\n");
        send_reply(500, 0, rtsp);    /* internal server error */
        if (r) 
        {
            free(r);
        }
        if (mb) 
        {
            free(mb);
        }
        return ERR_ALLOC;
    }

    /*构造describe消息串*/
    sprintf(r, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL"Server: %s/%s"RTSP_EL, RTSP_VER, 200, get_stat(200), rtsp->rtsp_cseq, PACKAGE, VERSION);
    add_time_stamp(r, 0);                 /*添加时间戳*/
    
    switch (descr_format) 
    {
        /*处理会话描述协议格式，如果添加新的SDP,在这里添加*/
        case df_SDP_format:
        {
            strcat(r, "Content-Type: application/sdp"RTSP_EL);   /*实体头，表示实体类型*/
            break;
        }
    }

    /*用于解析实体内相对url的 绝对url*/
    sprintf(r + strlen(r), "Content-Base: rtsp://%s/%s/"RTSP_EL, prefs_get_hostname(), object);
    sprintf(r + strlen(r), "Content-Length: %d"RTSP_EL, strlen(descr));    /*消息体的长度*/
    strcat(r, RTSP_EL);
    /*消息头结束*/

    /*加上消息体*/
    strcat(r, descr);    /*describe消息*/
    bwrite(r, (unsigned short) strlen(r), rtsp);     /*向缓冲区中填充数据*/

    free(mb);
    free(r);

    #ifdef RTSP_METHOD_LOG
    fnc_log(FNC_LOG_CLIENT,"200 %d %s ",strlen(descr),object);
    #endif
    
    return ERR_NOERROR;
}

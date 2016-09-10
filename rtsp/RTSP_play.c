/* * 
 *  $Id: RTSP_play.c 338 2006-04-27 16:45:52Z shawill $
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
#include <netinet/in.h>

#include <fenice/bufferpool.h>
#include <fenice/rtsp.h>
#include <fenice/utils.h>
#include <fenice/prefs.h>
#include <fenice/fnc_log.h>

#include "../config.h"
/*
****************************************************************
*PLAY 方法的处理
****************************************************************
*/

int RTSP_play(RTSP_buffer * rtsp)
{
    DEBUG_PRINT_INFO("prepare to play!\n");
    
    /*局部变量*/
    int url_is_file;
    char object[255], server[255], trash[255];
    char url[255];
    unsigned short port;
    char *p = NULL, *q = NULL;
    long int session_id;
    RTSP_session *ptr;
    RTP_session *ptr2;
    play_args args;
    int time_taken = 0;

    /*检查是否包含序列号头*/
    if ((p = strstr(rtsp->in_buffer, HDR_CSEQ)) == NULL) 
    {
        send_reply(400, 0, rtsp);   /* Bad Request */
        return ERR_NOERROR;
    } 
    else 
    {
        if (sscanf(p, "%254s %d", trash, &(rtsp->rtsp_cseq)) != 2) 
        {
            send_reply(400, 0, rtsp);    /* Bad Request */
            return ERR_NOERROR;
        }
    }
    
    /*获取播放范围*/
    args.playback_time_valid = 0;
    args.start_time_valid = 0;

    /*检查Range头*/
    if ((p = strstr(rtsp->in_buffer, HDR_RANGE)) != NULL) 
    {
        q = strstr(p, "npt");   /*处理NPT*/
        if (q == NULL) 
        {
            q = strstr(p, "smpte");  /*处理smpte*/
            if (q == NULL) 
            {
                q = strstr(p, "clock");
                
                if (q == NULL) 
                {
                    /*没有指定格式，使用 NeMeSI   格式*/                                   
                    if ((q = strstr(p, "time")) == NULL) 
                    {
                         
                        double t;
                        q = strstr(p, ":");
                        sscanf(q + 1, "%lf", &t);
                        
                        args.start_time = t * 60 * 60;
                        
                        /*Min*/
                        q = strstr(q + 1, ":");
                        sscanf(q + 1, "%lf", &t);
                        args.start_time += (t * 60);
                        
                        /*Sec*/ 
                        q = strstr(q + 1, ":");
                        sscanf(q + 1, "%lf", &t);
                        args.start_time += t;
                        args.start_time_valid = 1;
                    }
                    else      /*处理 time*/
                    {
                        args.start_time = 0;
                        args.end_time = 0;
                        time_taken = 1;
                    }
                } 
                else 
                {
                    /*不支持clock，使用默认值*/
                    args.start_time = 0;
                    args.end_time = 0;
                }
            } else
            {
                /*snmpte, 不支持，使用默认*/
                args.start_time = 0;
                args.end_time = 0;
            }
        }
        else
        {
            /*处理npt*/
            if ((q = strchr(q, '=')) == NULL) 
            {
                send_reply(400, 0, rtsp);/* Bad Request */
                return ERR_NOERROR;
            }
            sscanf(q + 1, "%f", &(args.start_time));          /*开始时间*/
            
            if ((q = strchr(q, '-')) == NULL) 
            {
                send_reply(400, 0, rtsp);/* Bad Request */
                return ERR_NOERROR;
            }
            if (sscanf(q + 1, "%f", &(args.end_time)) != 1)      /*结束时间*/
            {
                args.end_time = 0;                 /*play all the media,until ending ,yanf*/
            }
        }

        /*查找开始回放时间*/
        if ((q = strstr(p, "time")) == NULL) 
        {
            /*没有指定，立即回放*/
            memset(&(args.playback_time), 0, sizeof(args.playback_time));
        } 
        else 
        {
            if (!time_taken) 
            {
                q = strchr(q, '=');
                /*获得时间*/
                if (get_UTC_time(&(args.playback_time), q + 1) != ERR_NOERROR)
                {
                    memset(&(args.playback_time), 0, sizeof(args.playback_time));
                }
                args.playback_time_valid = 1;
            }
        }
    } 
    else
    {
        args.start_time = 0;
        args.end_time = 0;
        memset(&(args.playback_time), 0, sizeof(args.playback_time));
    }
    
    /*序列号*/
    if ((p = strstr(rtsp->in_buffer, HDR_CSEQ)) == NULL) 
    {
        send_reply(400, 0, rtsp);   /* Bad Request */
        return ERR_NOERROR;
    }
    
    /* If we get a Session hdr, then we have an aggregate control*/
    if ((p = strstr(rtsp->in_buffer, HDR_SESSION)) != NULL) 
    {
        if (sscanf(p, "%254s %ld", trash, &session_id) != 2) 
        {
            send_reply(454, 0, rtsp);/* Session Not Found */
            return ERR_NOERROR;
        }
    } 
    else 
    {
        send_reply(400, 0, rtsp);/* bad request */
        return ERR_NOERROR;
    }

    
    /*分离出URL */
    if (!sscanf(rtsp->in_buffer, " %*s %254s ", url)) 
    {
        send_reply(400, 0, rtsp);/* bad request */
        return ERR_NOERROR;
    }
    
    /* 验证URL的正确性*/
    switch (parse_url(url, server, sizeof(server), &port, object, sizeof(object)))
    {
        case 1: 
            send_reply(400, 0, rtsp);
            return ERR_NOERROR;
            break;
            
        case -1:
            send_reply(500, 0, rtsp);
            return ERR_NOERROR;
            break;
            
        default:
            break;
    }

    
    if (strcmp(server, prefs_get_hostname()) != 0) 
    {
    }

    /*禁止采用相对路径访问*/
    if (strstr(object, "../")) 
    {
        send_reply(403, 0, rtsp);   /* Forbidden */
        return ERR_NOERROR;
    }
    if (strstr(object, "./")) 
    {
        send_reply(403, 0, rtsp); /* Forbidden */
        return ERR_NOERROR;
    }


    p = strrchr(object, '.');
    url_is_file = 0;
    if (p == NULL) 
    {
        send_reply(415, 0, rtsp);   /* Unsupported media type */
        return ERR_NOERROR;
    } 
    else 
    {
        url_is_file = is_supported_url(p);
    }
    q = strchr(object, '!');
    /*查找!*/

    if (q == NULL) 
    {
        /* PLAY <file.sd>*/
        ptr = rtsp->session_list;
        if (ptr != NULL) 
        {
            if (ptr->session_id == session_id) 
            {
                /*查找RTP session*/ 
                for (ptr2 = ptr->rtp_session; ptr2 != NULL; ptr2 = ptr2->next) 
                {
                    if (ptr2->current_media->description.priority == 1)
                    {
                        /*播放所有演示*/
                        if (!ptr2->started) 
                        {
                            /*开始新的播放*/ 
                            DEBUG_PRINT_INFO("+++++++++++++++++++++")
                            DEBUG_PRINT_INFO("start to play now!");
                            DEBUG_PRINT_INFO("+++++++++++++++++++++")
                            
                            if (schedule_start(ptr2->sched_id, &args) == ERR_ALLOC)
                                return ERR_ALLOC;
                        } 
                        else
                        {
                            /*恢复暂停，播放*/ 
                            if (!ptr2->pause) 
                            {
                                //fnc_log(FNC_LOG_INFO,"PLAY: already playing\n");
                            } 
                            else 
                            {
                                schedule_resume(ptr2->sched_id, &args);
                            }
                        }
                    }
                }
            } 
            else 
            {
                send_reply(454, 0, rtsp);	/* Session not found*/
                return ERR_NOERROR;
            }
        } 
        else 
        {
            send_reply(415, 0, rtsp);  /* Internal server error*/
            return ERR_GENERIC;
        }
    }
    else
    {
        if (url_is_file) 
        {
            /*PLAY <file.sd>!<file>    */                     
            ptr = rtsp->session_list;
            if (ptr != NULL) 
            {
                if (ptr->session_id != session_id) 
                {
                    send_reply(454, 0, rtsp);   /*Session not found*/ 
                    return ERR_NOERROR;
                }
                /*查找RTP会话*/
                for (ptr2 = ptr->rtp_session; ptr2 != NULL; ptr2 = ptr2->next) 
                {
                    if (strcmp(ptr2->current_media->filename, q + 1) == 0) 
                    {
                        break;
                    }
                }
                
                if (ptr2 != NULL) 
                {
                    if (schedule_start(ptr2->sched_id, &args) == ERR_ALLOC)
                        return ERR_ALLOC;
                } 
                else 
                {
                    send_reply(454, 0, rtsp);/* Session not found*/
                    return ERR_NOERROR;
                }
            } 
            else
            {
                send_reply(415, 0, rtsp);	/*Internal server error*/ 
                return ERR_GENERIC;
            }
        } 
        else 
        {
            // PLAY <file.sd>!<aggr>
            ptr = rtsp->session_list;
            if (ptr != NULL) 
            {
                if (ptr->session_id != session_id) 
                {
                    send_reply(454, 0, rtsp);/* Session not found*/
                    return ERR_NOERROR;
                }
                /*控制集，播放所有的RTP*/ 
                for (ptr2 = ptr->rtp_session; ptr2 != NULL; ptr2 = ptr2->next) 
                {
                    if (!ptr2->started)
                    {
                        /*开始新的播放*/ 
                        DEBUG_PRINTF("#----++++++++++++++++--------schedule to playing now --------------+++++++++++\n");
                        if (schedule_start(ptr2->sched_id, &args) == ERR_ALLOC)
                            return ERR_ALLOC;
                    } 
                    else
                    {
                        /*恢复暂停，播放*/ 
                        if (!ptr2->pause) 
                        {
                            /*fnc_log(FNC_LOG_INFO,"PLAY: already playing\n");*/
                        } 
                        else 
                        {
                            schedule_resume(ptr2->sched_id, &args);
                        }
                    }
                }
            } 
            else 
            {
                send_reply(415, 0, rtsp);	// Internal server error
                return ERR_GENERIC;
            }
        }
    }

    /*发送回复消息*/ 
    send_play_reply(rtsp, object, ptr);
    
    #ifdef RTSP_METHOD_LOG   
    fnc_log(FNC_LOG_INFO,"PLAY %s RTSP/1.0 ",url); 
    // See User-Agent 
    if ((p=strstr(rtsp->in_buffer, HDR_USER_AGENT))!=NULL) 
    {
        char cut[strlen(p)];
        strcpy(cut,p);
        p=strstr(cut, "\n");
        cut[strlen(cut)-strlen(p)-1]='\0';
        fnc_log(FNC_LOG_CLIENT,"%s\n",cut);
    }
    else
        fnc_log(FNC_LOG_CLIENT,"- \n");
    #endif
        
    return ERR_NOERROR;
}

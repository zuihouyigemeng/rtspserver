/* * 
 *  $Id: RTSP_state_machine.c 133 2005-05-09 17:35:14Z federico $
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

#include <fenice/rtsp.h>
#include <fenice/utils.h>
#include <fenice/fnc_log.h>

#include "../config.h"

/*rtsp状态机，服务器端*/
void RTSP_state_machine(RTSP_buffer * rtsp, int method)
{
    DEBUG_PRINT_INFO("entering rtsp_state_machine!")

    /*除了播放过程中发送的最后一个数据流，
     *所有的状态迁移都在这里被处理
     * 状态迁移位于stream_event中
     */
    char *s;
    RTSP_session *p;
    long int session_id;
    char trash[255];

    char  szDebug[255];

    /*找到会话位置*/
    if ((s = strstr(rtsp->in_buffer, HDR_SESSION)) != NULL) 
    {
        if (sscanf(s, "%254s %ld", trash, &session_id) != 2) 
        {
            fnc_log(FNC_LOG_INFO,"Invalid Session number in Session header\n");
            send_reply(454, 0, rtsp);              /* 没有此会话*/
            return;
        }
    }

    /*打开会话列表*/
    p = rtsp->session_list;
    if (p == NULL) {
        return;
    }

    sprintf(szDebug,"state_machine:current state is  ");
    strcat(szDebug,((p->cur_state==0)?"init state":((p->cur_state==1)?"ready state":"play state")));
    DEBUG_PRINT_INFO(szDebug)
    
    /*根据状态迁移规则，从当前状态开始迁移*/
    switch (p->cur_state) 
    {
        case INIT_STATE:                    /*初始态*/
        {
            switch (method)
            {
                sprintf(szDebug,"current method code is:  %d  \n",method);
                DEBUG_PRINT_INFO(szDebug)
                
                case RTSP_ID_DESCRIBE:  /*状态不变*/
                    RTSP_describe(rtsp);
                    break;
                    
                case RTSP_ID_SETUP:                /*状态迁移为就绪态*/
                    if (RTSP_setup(rtsp, &p) == ERR_NOERROR) 
                    {
                        p->cur_state = READY_STATE;
                        DEBUG_PRINT_INFO("TRANSFER TO READY STATE!");
                    }
                    break;
                    
                case RTSP_ID_TEARDOWN:       /*状态不变*/
                    RTSP_teardown(rtsp);
                    break;
                    
                case RTSP_ID_OPTIONS:
                    if (RTSP_options(rtsp) == ERR_NOERROR)
                    {
                        p->cur_state = INIT_STATE;             /*状态不变*/                  
                    }
                    break;
               
                case RTSP_ID_PLAY:          /* method not valid this state. */
                
                case RTSP_ID_PAUSE:
                    send_reply(455, "Accept: OPTIONS, DESCRIBE, SETUP, TEARDOWN\n", rtsp);
                    break;
                    
                default:
                    send_reply(501, "Accept: OPTIONS, DESCRIBE, SETUP, TEARDOWN\n", rtsp);
                    break;
            }
        break;
        }                                                               
            
        case READY_STATE:
        {
            sprintf(szDebug,"current method code is:%d\n",method);
            DEBUG_PRINT_INFO(szDebug)
            
            switch (method) 
            {
                case RTSP_ID_PLAY:                                      /*状态迁移为播放态*/
                    if (RTSP_play(rtsp) == ERR_NOERROR) 
                    {
                        DEBUG_PRINT_INFO("playing no error!\n");
                        p->cur_state = PLAY_STATE;
                    }
                    break;
                    
                case RTSP_ID_SETUP:                                         
                    if (RTSP_setup(rtsp, &p) == ERR_NOERROR)       /*状态不变*/   
                    {
                        p->cur_state = READY_STATE;
                    }
                    break;
                
                case RTSP_ID_TEARDOWN:
                    RTSP_teardown(rtsp);                                         /*状态变为初始态 ?*/
                    break;
                
                case RTSP_ID_OPTIONS:
                    if (RTSP_options(rtsp) == ERR_NOERROR)
                    {
                        p->cur_state = INIT_STATE;             /*状态不变*/                  
                    }
                    break;
                    
                case RTSP_ID_PAUSE:                                             /* method not valid this state. */
                    send_reply(455, "Accept: OPTIONS, SETUP, PLAY, TEARDOWN\n", rtsp);
                    break;

                case RTSP_ID_DESCRIBE:
                    RTSP_describe(rtsp);
                    break;

                default:
                    send_reply(501, "Accept: OPTIONS, SETUP, PLAY, TEARDOWN\n", rtsp);
                    break;
            }
            
            break;
        }


        case PLAY_STATE:
        {
            switch (method)
            {
                case RTSP_ID_PLAY:
                    // Feature not supported
                    fnc_log(FNC_LOG_INFO,"UNSUPPORTED: Play while playing.\n");
                    send_reply(551, 0, rtsp);        /* Option not supported */
                    break;

                case RTSP_ID_PAUSE:                                  /*状态迁移为就绪态*/
                    if (RTSP_pause(rtsp) == ERR_NOERROR)
                    {
                        DEBUG_PRINT_INFO("pause  no error!\n")                 
                        p->cur_state = READY_STATE;    
                    }
                    break;

                case RTSP_ID_TEARDOWN:
                    RTSP_teardown(rtsp);                             /*状态迁移为初始态*/
                    break;
                
                case RTSP_ID_OPTIONS:
                    break;
                
                case RTSP_ID_DESCRIBE:
                    RTSP_describe(rtsp);
                    break;
                
                case RTSP_ID_SETUP:
                    break;
            }
            
            break;
        }/* PLAY state */

        default:
            {        
                /* invalid/unexpected current state. */
                fnc_log(FNC_LOG_ERR,"State error: unknown state=%d, method code=%d\n", p->cur_state, method);
            }
            break;
    }/* end of current state switch */

    DEBUG_PRINT_INFO("leaving rtsp_state_machine!")
}

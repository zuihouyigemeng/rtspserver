/* * 
 *  $Id: RTSP_setup.c 351 2006-06-01 17:58:07Z shawill $
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
#include <sys/time.h>              // shawill: for gettimeofday
#include <stdlib.h>                  // shawill: for rand, srand
#include <unistd.h>                   // shawill: for dup
// shawill: for inet_aton
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fenice/rtsp.h>
#include <fenice/socket.h>
#include <fenice/utils.h>
#include <fenice/prefs.h>
#include <fenice/multicast.h>
#include <fenice/fnc_log.h>

#include "../config.h"
/*
****************************************************************
*SETUP 方法的处理
****************************************************************
*/


int RTSP_setup(RTSP_buffer * rtsp, RTSP_session ** new_session)
{
    /*局部变量*/
    char address[16];
    char object[255], server[255];
    char url[255];
    unsigned short port;
    RTSP_session *rtsp_s;
    RTP_session *rtp_s, *rtp_s_prec;
    int SessionID = 0;

    
    struct timeval now_tmp;
    char *p;
    unsigned int start_seq, start_rtptime;
    char transport_str[255];
    media_entry *list, *matching_me, req;
    struct sockaddr_storage rtsp_peer;
    socklen_t namelen = sizeof(rtsp_peer);
    unsigned long ssrc;
    SD_descr *matching_descr;
    unsigned char is_multicast_dad = 1;  
    RTP_transport transport;
    char *saved_ptr, *transport_tkn;
    int max_interlvd;

    char szDebug[128];

    /*变量初始化*/
    memset(&req, 0, sizeof(req));
    memset(&transport, 0, sizeof(transport));

    /*解析分离输入消息*/ 

    /* 得到URL */
    if (!sscanf(rtsp->in_buffer, " %*s %254s ", url))
    {
        send_reply(400, 0, rtsp);   /* bad request */
        return ERR_NOERROR;
    }
    
    /* 验证URL的合法性 */
    switch (parse_url(url, server, sizeof(server), &port, object, sizeof(object))) 
    {
        case 1:                /*非法的请求*/
            send_reply(400, 0, rtsp);             
            return ERR_NOERROR;
            
        case -1:     /*服务器内部错误*/
            send_reply(500, 0, rtsp);
            return ERR_NOERROR;
            break;
            
        default:
            break;
    }
    if (strcmp(server, prefs_get_hostname()) != 0) 
    {
    /* Currently this feature is disabled. */
    /* wrong server name */
    /*      send_reply(404, 0 , rtsp);
            return ERR_NOERROR;
     */
    }

    /*禁止使用相对路径名*/
    if (strstr(object, "../")) 
    {

        send_reply(403, 0, rtsp);	/* Forbidden */
        return ERR_NOERROR;
    }    
    if (strstr(object, "./")) 
    {
        send_reply(403, 0, rtsp);   /* Forbidden */
        return ERR_NOERROR;
    }

    /*根据后缀判断媒体类型*/
    if (!(p = strrchr(object, '.')))
    {   
        /*无后缀*/
        send_reply(415, 0, rtsp);/* Unsupported media type */
        return ERR_NOERROR;
    } else 
        if ( !(p = strchr(object, '!')) ) 
        {
            /*if '!' is not present then a file has not been specified*/
            send_reply(500, 0, rtsp);/* Internal server error */
            return ERR_NOERROR;
        } else
        {
            // SETUP name.sd!stream
            strcpy(req.filename, p + 1);
            req.flags |= ME_FILENAME;

            *p = '\0';
        }

    {
        char temp[255];
        char *pd=NULL;

        strcpy(temp, object);
        
#if 0
printf("%s\n", object);
	// BEGIN 
	// if ( (p = strstr(temp, "/")) ) {
	if ( (p = strchr(temp, '/')) ) {
		strcpy(object, p + 1);	// CRITIC. 
	}
printf("%s\n", temp);
#endif

        pd = strstr(temp, ".sd");    /*查找后缀sd*/
        if ( (p = strstr(pd + 1, ".sd")) )
        {           
            /* have compatibility with RealOne*/
            strcpy(object, pd + 4);// CRITIC. 
        }               //Note: It's a critic part
    }

    /*just for debug,yanf*/
    /*DEBUG_PRINTF("+++++++++++++++++++++++++checking media!+++++++++++++++++++++++++++\n");*/
        
    /*查看媒体是否正确*/
    if (enum_media(object, &matching_descr) != ERR_NOERROR) 
    {
        send_reply(500, 0, rtsp);/* Internal server error */
        return ERR_NOERROR;
    }
    list=matching_descr->me_list;

    if (get_media_entry(&req, list, &matching_me) == ERR_NOT_FOUND)
    {
        send_reply(404, 0, rtsp);       /* Not found */
        return ERR_NOERROR;
    }

    
    /*just for debug,yanf*/
    /*DEBUG_PRINTF("+++++++++++++++++++++++++checking CSeq!+++++++++++++++++++++++++++\n");*/

    /* Get the CSeq */
    if ((p = strstr(rtsp->in_buffer, HDR_CSEQ)) == NULL) 
    {
        send_reply(400, 0, rtsp);       /* Bad Request */
        return ERR_NOERROR;
    } else {
        if (sscanf(p, "%*s %d", &(rtsp->rtsp_cseq)) != 1) 
        {
            send_reply(400, 0, rtsp);/* Bad Request */
            return ERR_NOERROR;
        }
    }
    
        /*if ((p = strstr(rtsp->in_buffer, "ssrc")) != NULL) {
        p = strchr(p, '=');
        sscanf(p + 1, "%lu", &ssrc);
        } else {*/
        
    ssrc = random32(0);

    /*just for debug,yanf*/  
    /*DEBUG_PRINTF("--------------------------------ssrc got!\n");*/
        //}

    /* Start parsing the Transport header*/
    if ((p = strstr(rtsp->in_buffer, HDR_TRANSPORT)) == NULL) 
    {
        send_reply(406, "Require: Transport settings" /* of rtp/udp;port=nnnn. "*/, rtsp);     /* Not Acceptable */
        return ERR_NOERROR;
    }

    /*检查传输层子串是否正确*/
    if (sscanf(p, "%*10s%255s", transport_str) != 1) 
    {
        fnc_log(FNC_LOG_ERR,"SETUP request malformed: Transport string is empty\n");
        send_reply(400, 0, rtsp);       /* Bad Request */
        return ERR_NOERROR;
    }

    /*just for debug,yanf*/
    /*DEBUG_PRINTF("+++++++++++++++++++++++++pasering transport list!+++++++++++++++++++++++++++\n");*/
    sprintf(szDebug,"transport: %s\n", transport_str);
    DEBUG_PRINT_INFO(szDebug)

    /*分离逗号分割的传输设置列表*/
    if ( !(transport_tkn=strtok_r(transport_str, ",", &saved_ptr)) ) 
    {
        fnc_log(FNC_LOG_ERR,"Malformed Transport string from client\n");
        send_reply(400, 0, rtsp);/* Bad Request */
        return ERR_NOERROR;
    }

    if (getpeername(rtsp->fd, (struct sockaddr *)&rtsp_peer, &namelen) != 0) 
    {
        send_reply(415, 0, rtsp);/*服务器内部错误*/
        return ERR_GENERIC;
    }

    transport.type = RTP_no_transport;

    
    do 
    { 
        if ( (p = strstr(transport_tkn, RTSP_RTP_AVP)) ) 
        { 
            /*Transport: RTP/AVP*/ 
            p += strlen(RTSP_RTP_AVP);
            if ( !*p || (*p == ';') || (*p == ' ')) 
            {
#if 0
                // if ((p = strstr(rtsp->in_buffer, "client_port")) == NULL && strstr(rtsp->in_buffer, "multicast") == NULL) {
                if ((p = strstr(transport_tkn, "client_port")) == NULL && strstr(transport_tkn, "multicast") == NULL) {
                send_reply(406, "Require: Transport settings of rtp/udp;port=nnnn. ", rtsp);	/* Not Acceptable */
                return ERR_NOERROR;
                }
#endif 
                /*单播*/
                if (strstr(transport_tkn, "unicast")) 
                {
                    /*如果指定了客户端端口号，填充对应的两个端口号*/
                    if( (p = strstr(transport_tkn, "client_port")) ) 
                    {
                        p = strstr(p, "=");
                        sscanf(p + 1, "%d", &(transport.u.udp.cli_ports.RTP));
                        p = strstr(p, "-");
                        sscanf(p + 1, "%d", &(transport.u.udp.cli_ports.RTCP));
                    }

                    /*服务器端口*/
                    if (RTP_get_port_pair(&transport.u.udp.ser_ports) != ERR_NOERROR) 
                    {
                        send_reply(500, 0, rtsp);/* Internal server error */
                        return ERR_GENERIC;
                    }

                    /*just for debug,yanf*/
                    /*DEBUG_PRINTF("+++++++++++++++++++++++++begin to send packet!+++++++++++++++++++++++++++\n");*/

                    /*发出RTP数据包的UDP连接*/
                    udp_connect(transport.u.udp.cli_ports.RTP, &transport.u.udp.rtp_peer, (*((struct sockaddr_in *) (&rtsp_peer))).sin_addr.s_addr,&transport.rtp_fd);
                    /*发出RTCP数据包的UDP连接*/
                    udp_connect(transport.u.udp.cli_ports.RTCP, &transport.u.udp.rtcp_out_peer,(*((struct sockaddr_in *) (&rtsp_peer))).sin_addr.s_addr, &transport.rtcp_fd_out);
                    udp_open(transport.u.udp.ser_ports.RTCP, &transport.u.udp.rtcp_in_peer, &transport.rtcp_fd_in);	//bind

                    transport.u.udp.is_multicast = 0;
                } 
                else if ( matching_descr->flags & SD_FL_MULTICAST )
                    {
                        /*multicast*/
                        /* TODO: make the difference between only multicast allowed or unicast fallback allowed.*/
                        transport.u.udp.cli_ports.RTP = transport.u.udp.ser_ports.RTP =matching_me->rtp_multicast_port;
                        transport.u.udp.cli_ports.RTCP = transport.u.udp.ser_ports.RTCP =matching_me->rtp_multicast_port+1;

                        is_multicast_dad = 0;
                        if (!(matching_descr->flags & SD_FL_MULTICAST_PORT) ) 
                        {
                            struct in_addr inp;
                            unsigned char ttl=DEFAULT_TTL;
                            struct ip_mreq mreq;

                            mreq.imr_multiaddr.s_addr = inet_addr(matching_descr->multicast);
                            mreq.imr_interface.s_addr = INADDR_ANY;
                            
                            setsockopt(transport.rtp_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
                            setsockopt(transport.rtp_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

                            is_multicast_dad = 1;
                            strcpy(address, matching_descr->multicast);
                            inet_aton(address, &inp);

                            /*分别发出RTP和RTCP数据报*/
                            udp_connect(transport.u.udp.ser_ports.RTP, &transport.u.udp.rtp_peer, inp.s_addr, &transport.rtp_fd);
                            inet_aton(address, &inp);
                            udp_connect(transport.u.udp.ser_ports.RTCP, &transport.u.udp.rtcp_out_peer, inp.s_addr, &transport.rtcp_fd_out);

                            if(matching_me->next==NULL)
                                matching_descr->flags |= SD_FL_MULTICAST_PORT;

                            matching_me->rtp_multicast_port = transport.u.udp.ser_ports.RTP;
                            transport.u.udp.is_multicast = 1;
                            
                            #ifdef RTSP_METHOD_LOG
                            fnc_log(FNC_LOG_DEBUG,"\nSet up socket for multicast ok\n");
                            #endif
                        }
                    } else
                continue;

                transport.type = RTP_rtp_avp;
                break; 
            } 
            else if (!strncmp(p, "/TCP", 4)) 
                { 
                    /*Transport: RTP/AVP/TCP;interleaved=x-y    XXX still not finished   */
                    if( (p = strstr(transport_tkn, "interleaved")) ) 
                    {
                        p = strstr(p, "=");
                        sscanf(p + 1, "%d", &(transport.u.tcp.interleaved.RTP));
                        if ( (p = strstr(p, "-")) )
                        sscanf(p + 1, "%d", &(transport.u.tcp.interleaved.RTCP));
                        else
                        transport.u.tcp.interleaved.RTCP = transport.u.tcp.interleaved.RTP + 1;
                    } 
                    else 
                    { 
                        /*查找交叉存取的最大通道号*/ 
                        max_interlvd = -1;
                        for (rtp_s = (rtsp->session_list)?rtsp->session_list->rtp_session:NULL; rtp_s; rtp_s = rtp_s->next)
                            max_interlvd = max(max_interlvd, ((rtp_s->transport.type == RTP_rtp_avp_tcp)?rtp_s->transport.u.tcp.interleaved.RTCP:-1));

                        transport.u.tcp.interleaved.RTP = max_interlvd + 1;
                        transport.u.tcp.interleaved.RTCP = max_interlvd + 2;
                    }

                    transport.rtp_fd = rtsp->fd;
                    transport.rtcp_fd_out = rtsp->fd; 
                    transport.rtcp_fd_in = -1;

                    transport.type = RTP_rtp_avp_tcp;
                    break; 
                }
                
        }
    } while ((transport_tkn=strtok_r(NULL, ",", &saved_ptr)));
    
    sprintf(szDebug,"rtp transport: %d\n", transport.type);
    DEBUG_PRINT_INFO(szDebug)
    

    /*不支持的传输类型*/
    if (transport.type == RTP_no_transport) 
    {
        // fnc_log(FNC_LOG_ERR,"Unsupported Transport\n");
        send_reply(461, "Unsupported Transport", rtsp);/* Bad Request */
        return ERR_NOERROR;
    }

    /*如果有会话头，就有了一个控制集合 */ 
    if ((p = strstr(rtsp->in_buffer, HDR_SESSION)) != NULL) 
    {
        if (sscanf(p, "%*s %d", &SessionID) != 1) 
        {
            send_reply(454, 0, rtsp); /* Session Not Found */
            return ERR_NOERROR;
        }
    }
    else
    {
        /*产生一个非0的随机的会话序号*/ 
        gettimeofday(&now_tmp, 0);
        srand((now_tmp.tv_sec * 1000) + (now_tmp.tv_usec / 1000));
#ifdef WIN32
        SessionID = rand();
#else
        SessionID = 1 + (int) (10.0 * rand() / (100000 + 1.0));
#endif
        if (SessionID == 0) 
        {
            SessionID++;
        }
    }

    /*如果需要增加一个会话*/ 
    if ( !rtsp->session_list ) 
    {
        rtsp->session_list = (RTSP_session *) calloc(1, sizeof(RTSP_session));
    }
    rtsp_s = rtsp->session_list;

    /*建立一个新会话，插入到链表中*/ 
    if (rtsp->session_list->rtp_session == NULL) 
    {
        rtsp->session_list->rtp_session = (RTP_session *) calloc(1, sizeof(RTP_session));
        rtp_s = rtsp->session_list->rtp_session;
    } 
    else 
    {
        for (rtp_s = rtsp_s->rtp_session; rtp_s != NULL; rtp_s = rtp_s->next) 
        {
            rtp_s_prec = rtp_s;
        }
        rtp_s_prec->next = (RTP_session *) calloc(1, sizeof(RTP_session));
        rtp_s = rtp_s_prec->next;
    }


#ifdef WIN32
    start_seq = rand();
    start_rtptime = rand();
#else
    // start_seq = 1 + (int) (10.0 * rand() / (100000 + 1.0));
    // start_rtptime = 1 + (int) (10.0 * rand() / (100000 + 1.0));
#if 0
    start_seq = 1 + (unsigned int) ((float)(0xFFFF) * ((float)rand() / (float)RAND_MAX));
    start_rtptime = 1 + (unsigned int) ((float)(0xFFFFFFFF) * ((float)rand() / (float)RAND_MAX));
#else
    start_seq = 1 + (unsigned int) (rand()%(0xFFFF));
    start_rtptime = 1 + (unsigned int) (rand()%(0xFFFFFFFF));
#endif
#endif

    /*填写会话中的变量*/
    if (start_seq == 0) 
    {
        start_seq++;
    }
    if (start_rtptime == 0)
    {
        start_rtptime++;
    }
    rtp_s->pause = 1;
    strcpy(rtp_s->sd_filename, object);

    /*为Media分配空间*/
    rtp_s->current_media = (media_entry *) calloc(1, sizeof(media_entry));

    if( is_multicast_dad ) 
    {
        if ( mediacpy(&rtp_s->current_media, &matching_me) ) 
        {
            send_reply(500, 0, rtsp);/* Internal server error */
            return ERR_GENERIC;
        }
    }

    /*设置随机数产生器的种子*/
    gettimeofday(&now_tmp, 0);
    srand((now_tmp.tv_sec * 1000) + (now_tmp.tv_usec / 1000));
    
    rtp_s->start_rtptime = start_rtptime;
    rtp_s->start_seq = start_seq;
    memcpy(&rtp_s->transport, &transport, sizeof(transport));
    rtp_s->is_multicast_dad = is_multicast_dad;
    rtp_s->sd_descr=matching_descr;
    rtp_s->sched_id = schedule_add(rtp_s);
    rtp_s->ssrc = ssrc;
    

    rtsp_s->session_id = SessionID;
    *new_session = rtsp_s;
    
    /*建立会话，发送会话*/
    #ifdef RTSP_METHOD_LOG
    fnc_log(FNC_LOG_INFO,"SETUP %s RTSP/1.0 ",url);
    #endif
    send_setup_reply(rtsp, rtsp_s, matching_descr, rtp_s);


    
    /*将USER-AGENT写入到日志中*/ 
    #ifdef RTSP_METHOD_LOG
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

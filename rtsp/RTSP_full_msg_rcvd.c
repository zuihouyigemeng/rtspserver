/* * 
 *  $Id: RTSP_full_msg_rcvd.c 350 2006-05-31 18:22:15Z shawill $
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

/*根据缓冲区的内容，填充后边两个长度数据,检查缓冲区中消息的完整性
 * return -1 on ERROR
 * return RTSP_not_full (0) if a full RTSP message is NOT present in the in_buffer yet.
 * return RTSP_method_rcvd (1) if a full RTSP message is present in the in_buffer and is
 *                     ready to be handled.
 * return RTSP_interlvd_rcvd (2) if a complete RTP/RTCP interleaved packet is present.  
 * terminate on really ugly cases.
 */
int RTSP_full_msg_rcvd(RTSP_buffer *rtsp, int *hdr_len, int *body_len)
/* This routine is from OMS.*/
{
    int eomh;    /* end of message header found */
    int mb;       /* message body exists */
    int tc;         /* terminator count */
    int ws;        /* white space */
    unsigned int ml;              /* total message length including any message body */
    int bl;                           /* message body length */
    char c;                         /* character */
    int control;
    char *p;

    /*是否存在交叉存取的二进制rtp/rtcp数据包，参考RFC2326-10.12*/
    if (rtsp->in_buffer[0] == '$') 
    {
        uint16 *intlvd_len = (uint16 *)&rtsp->in_buffer[2];   /*跳过通道标志符*/

        /*转化为主机字节序，因为长度是网络字节序*/
        if ( (bl = ntohs(*intlvd_len)) <= rtsp->in_size)           
        {
            // fnc_log(FNC_LOG_DEBUG,"Interleaved RTP or RTCP packet arrived (len: %hu).\n", bl);
            if (hdr_len)
                *hdr_len = 4;
            if (body_len)
                *body_len = bl;
            return RTSP_interlvd_rcvd;
        }
        else
        {
            /*缓冲区不能完全存放数据*/
            fnc_log(FNC_LOG_DEBUG,"Non-complete Interleaved RTP or RTCP packet arrived.\n");
            return RTSP_not_full;
        }
        
    }

    
    eomh = mb = ml = bl = 0;
    while (ml <= rtsp->in_size)
    {
        /* look for eol. */
        /*计算不包含回车、换行在内的所有字符数*/
        control = strcspn(&(rtsp->in_buffer[ml]), "\r\n");
        if(control > 0)
            ml += control;
        else
            return ERR_GENERIC;

        /* haven't received the entire message yet. */
        if (ml > rtsp->in_size)
            return RTSP_not_full;        

            
        /* 处理终结符，判读是否是消息头的结束*/
        tc = ws = 0;
        while (!eomh && ((ml + tc + ws) < rtsp->in_size)) 
        {
            c = rtsp->in_buffer[ml + tc + ws];
            /*统计回车换行*/
            if (c == '\r' || c == '\n')
                tc++;                                         
            else if ((tc < 3) && ((c == ' ') || (c == '\t')))
            {
                ws++;                           /*回车、换行之间的空格或者TAB，也是可以接受的 */
            }
            else
            {
                    break;
            }
        }
        
        /*
         *一对回车、换行符仅仅被统计为一个行终结符
         * 双行可以被接受，并将其认为是消息头的结束标识
         * 这与RFC2068中的描述一致，参考rfc2068 19.3
         *否则，对所有的HTTP/1.1兼容协议消息元素来说，
         *回车、换行被认为是合法的行终结符
         */

        /* must be the end of the message header */
        if ((tc > 2) || ((tc == 2) && (rtsp->in_buffer[ml] == rtsp->in_buffer[ml + 1])))
            eomh = 1;                                
        ml += tc + ws;

        if (eomh)
        {
            ml += bl;   /* 加入消息体长度 */
            if (ml <= rtsp->in_size)
                break;  /* all done finding the end of the message. */
        }

        if (ml >= rtsp->in_size)
            return RTSP_not_full;   /* 还没有完全接收消息 */

        /*检查每一行的第一个记号，确定是否有消息体存在 */
        if (!mb)
        {          
            /* content length token not yet encountered. */
            if (!strncasecmp(&(rtsp->in_buffer[ml]), HDR_CONTENTLENGTH, strlen(HDR_CONTENTLENGTH))) 
            {
                mb = 1;                        /* 存在消息体. */
                ml += strlen(HDR_CONTENTLENGTH);

                /*跳过:和空格，找到长度字段*/
                while (ml < rtsp->in_size) 
                {
                    c = rtsp->in_buffer[ml];
                    if ((c == ':') || (c == ' '))
                        ml++;                  
                    else
                        break;
                }

                if (sscanf(&(rtsp->in_buffer[ml]), "%d", &bl) != 1) 
                {
                    fnc_log(FNC_LOG_ERR,"RTSP_full_msg_rcvd(): Invalid ContentLength encountered in message.\n");
                    return ERR_GENERIC;
                }
            }
        }
    }

    if (hdr_len)
        *hdr_len = ml - bl;

    if (body_len) {
    /*
     * go through any trailing nulls.  Some servers send null terminated strings
     * following the body part of the message.  It is probably not strictly
     * legal when the null byte is not included in the Content-Length count.
     * However, it is tolerated here.
     */
        for (tc = rtsp->in_size - ml, p = &(rtsp->in_buffer[ml]); tc && (*p == '\0'); p++, bl++, tc--);
            *body_len = bl;
    }

    return RTSP_method_rcvd;
}

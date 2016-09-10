/* * 
 *  $Id: RTP_sendto.c 352 2006-06-05 17:10:07Z shawill $
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

#include <fenice/rtp.h>
#include <fenice/types.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

ssize_t RTP_sendto(RTP_session *session, rtp_protos proto, unsigned char *pkt, ssize_t pkt_size)
{
    ssize_t sent = -1;
    tsocket fd = (proto == rtp_proto) ? session->transport.rtp_fd : session->transport.rtcp_fd_out;
    
    struct sockaddr *peer = (proto == rtp_proto) ? &(session->transport.u.udp.rtp_peer) : &(session->transport.u.udp.rtcp_out_peer);
    
    socklen_t peer_len = (proto == rtp_proto) ? sizeof(session->transport.u.udp.rtp_peer) : sizeof(session->transport.u.udp.rtcp_out_peer);

    switch (session->transport.type) 
    {
        case RTP_rtp_avp:
            sent = sendto(fd, pkt,pkt_size,0, peer, peer_len);
            break;
        
        case RTP_rtp_avp_tcp:
            {
                char tcp_pkt[pkt_size + 4];
                uint16 *intlvd_ch = (uint16 *)&tcp_pkt[2];

                tcp_pkt[0]='$';
                tcp_pkt[1]=(proto == rtp_proto) ? (char)session->transport.u.tcp.interleaved.RTP : (char)session->transport.u.tcp.interleaved.RTCP;
                *intlvd_ch = htons((uint16)pkt_size);
                memcpy(tcp_pkt + 4, pkt, pkt_size);
                
                if (fd>0)
                    sent = send(fd, tcp_pkt, pkt_size + 4, MSG_NOSIGNAL /*0*/);
                break;
            }
        
        default:
        break;
    }

    return sent;
}

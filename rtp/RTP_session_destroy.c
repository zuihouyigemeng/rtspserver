/* * 
 *  $Id: RTP_session_destroy.c 351 2006-06-01 17:58:07Z shawill $
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

#include <config.h>

/*#if HAVE_ALLOCA_H
#include <alloca.h>
#endif*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fenice/rtp.h>
#include <fenice/utils.h>
#include <fenice/bufferpool.h>

RTP_session *RTP_session_destroy(RTP_session *session)
{
    RTP_session *next = session->next;
    OMSBuffer *buff = session->current_media->pkt_buffer;
    /*struct stat fdstat;*/

    /* shawill: close socket. To be moved in a specific function (RTP_transport_close)
    Release SD_flag using in multicast and unjoing the multicast group
    if(session->sd_descr->flags & SD_FL_MULTICAST){  
    */
        
    if( (session->transport.type==RTP_rtp_avp) && session->transport.u.udp.is_multicast) 
    {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(session->sd_descr->multicast);
        mreq.imr_interface.s_addr = INADDR_ANY;
        setsockopt(session->transport.rtp_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
        session->sd_descr->flags &= ~SD_FL_MULTICAST_PORT;          /*Release SD_FL_MULTICAST_PORT*/
    }

    switch (session->transport.type)
    {
        case RTP_rtp_avp:
            // we must close socket only if we use udp, because for interleaved tcp
            // we use rtsp that must not be closed here.
            close(session->transport.rtp_fd);
            close(session->transport.rtcp_fd_in);
            close(session->transport.rtcp_fd_out);
            // release ports
            RTP_release_port_pair(&(session->transport.u.udp.ser_ports));
            break;

        case RTP_rtp_avp_tcp:
            session->transport.rtp_fd = session->transport.rtcp_fd_out = -1;
            break;
            
        default:
            break;
    }

    // destroy consumer
    OMSbuff_unref(session->cons);

    /*如果使用共享内存的消费者为0，则删除该缓存*/
    if (session->current_media->pkt_buffer->control->refs==0)
    {
        session->current_media->pkt_buffer=NULL;
        OMSbuff_free(buff);
        // close file if it's not a pipe
        //fstat(session->current_media->fd, &fdstat);
        //if ( !S_ISFIFO(fdstat.st_mode) )
        mediaclose(session->current_media);
    }
    
    // Deallocate memory
    free(session);

    return next;
}

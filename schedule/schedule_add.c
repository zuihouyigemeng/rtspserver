/* * 
 *  $Id: schedule_add.c 94 2005-02-04 13:53:15Z federico $
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

#include <fenice/schedule.h>
#include <fenice/rtp.h>
#include <fenice/utils.h>

#include "../config.h"

extern schedule_list sched[ONE_FORK_MAX_CONNECTION];

int schedule_add(RTP_session *rtp_session/*,RTSP_session *rtsp_session*/)
{
    int i;
    for (i=0; i<ONE_FORK_MAX_CONNECTION; ++i) 
    {
        /*需是还没有被加入到调度队列中的会话*/
        if (!sched[i].valid) 
        {
            sched[i].valid=1;
            sched[i].rtp_session=rtp_session;
            
            //sched[i].rtsp_session=rtsp_session;
            if(rtp_session->is_multicast_dad)                    /*如果是多播组中的第一个*/
            {
                sched[i].play_action=RTP_send_packet;
                DEBUG_PRINTF("<><><><><>adding a schedule object,action is rtp_send_packet!\n");
            }
                
            return i;
        }
    }
    // if (i >= MAX_SESSION) {
    return ERR_GENERIC;
    // }
}


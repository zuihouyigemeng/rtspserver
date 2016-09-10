/* * 
 *  $Id: rtcp.h 2 2004-06-08 15:08:53Z mancho $
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

#ifndef _RTCPH
#define _RTCPH

#include <fenice/rtp.h>

//#define MAX_PKT_SIZE 548 /* 576 - 20 - 8 = Minimum Reassembly Buffer Size - IP datagram header -  UDP header: in octects */
//#define MAX_SDES_LEN 255 /* in octects */

/*#define RTCP_MIN_TIME 5.0
#define RTCP_SENDER_BW_FRACTION 0.25
#define RTCP_RCVR_BW_FRACTION 0.75
#define COMPENSATION 1.21828*/ /* e - 1.5 */

typedef enum 
{
    /*sender report,for transmission and reception statics from participants that are active senders*/
    SR=200,               
    /*receiver report,for reception statistics from participants that are not active senders 
       and in combination with SR for    active senders reporting on more than 31 sources
     */
    RR=201,      
    SDES=202,/*Source description items, including CNAME,NAME,EMAIL,etc*/    
    BYE=203,/*Indicates end of participation*/
    APP=204/*Application-specific functions*/
} rtcp_pkt_type;

typedef enum 
{
    CNAME=1,    /*Canonical End-Point Identifier SDES Item*/
    NAME=2,     /*User Name SDES Item*/
    EMAIL=3,    /*User Name SDES Item*/
    PHONE=4,    /*Phone Number SDES Item*/
    LOC=5,          /*Geographic User Location SDES Item*/
    TOOL=6,     /*Application or Tool Name SDES Item*/
    NOTE=7,     /*Notice/Status SDES Item*/
    PRIV=8      /*Private Extensions SDES Item*/
} rtcp_info;

typedef struct _RTCP_header 
{
#if (BYTE_ORDER == LITTLE_ENDIAN)
    unsigned int count:5;       /*the number of blocks,0 is invalid*/
    unsigned int padding:1;   /*may be needed by some cryption algorithm*/
    unsigned int version:2;   /*Identifies the version of RTP, which is the same in RTCP packets    as in RTP data packets*/
#elif (BYTE_ORDER == BIG_ENDIAN)
    unsigned int version:2;
    unsigned int padding:1;
    unsigned int count:5;  //SC oppure RC
#else
#error Neither big nor little
#endif
    unsigned int pt:8;                    /*payload type*/
    unsigned int length:16;           /*the length of this packets in 32-bit words minus 1 */
} RTCP_header;

typedef struct _RTCP_header_SR 
{
    unsigned int ssrc;                        /*SSRC of sender*/
    unsigned int ntp_timestampH;    /*NTP timestamp, most significant word*/
    unsigned int ntp_timestampL;    /*NTP timestamp, least significant word*/
    unsigned int rtp_timestamp;      /*RTP timestamp*/
    unsigned int pkt_count;             /*sender's packet count*/
    unsigned int octet_count;           /*sender's octet count*/ 
} RTCP_header_SR;

typedef struct _RTCP_header_RR {
unsigned int ssrc;                              /*ssrc of receiver*/
} RTCP_header_RR;

typedef struct _RTCP_header_SR_report_block 
{
    unsigned long ssrc;
    unsigned char fract_lost;
    unsigned char pck_lost[3];                 /*cumulative number of packets lost*/
    unsigned int h_seq_no;                       /*extended highest sequence number received*/
    unsigned int jitter;                              /*interarrival jitter*/
    unsigned int last_SR;                          /*most recent  send report timestamps*/
    unsigned int delay_last_SR;               /*delay since last SR*/
} RTCP_header_SR_report_block;    

typedef struct _RTCP_header_SDES {
    unsigned int ssrc;                                
    unsigned char attr_name;
    unsigned char len;               
} RTCP_header_SDES;

typedef struct _RTCP_header_BYE {
    unsigned int ssrc;
    unsigned char length;
} RTCP_header_BYE;

int RTCP_send_packet(RTP_session *session,rtcp_pkt_type type);
int RTCP_recv_packet(RTP_session *session);
int RTCP_handler(RTP_session *session);

int RTCP_get_pkt_lost(RTP_session *session);
// Quanti pacchetti (RTP) ha perso il client

float RTCP_get_fract_lost(RTP_session *session);
// Quanti byte ha perso il client

unsigned int RTCP_get_jitter(RTP_session *session);
// Il jitter

unsigned int RTCP_get_RR_received(RTP_session *session);
// Quanti pacchetti RR ha ricevuto il server dal client

unsigned int RTCP_get_total_packet(RTP_session *session);
// Quanti pacchetti ha spedito il server

unsigned int RTCP_get_total_payload_octet(RTP_session *session);
// Quanti byte ha spedito il server

// int RTCP_parse(char *buff,RTP_session *session);
// Effettua il parsing di un pacchetto RTCP

// int RTCP_send_SDES(RTP_session *session);

int RTCP_get_pkt_lost(RTP_session *session);
// Quanti pacchetti (RTP) ha perso il client

float RTCP_get_fract_lost(RTP_session *session);
// Quanti byte ha perso il client

unsigned int RTCP_get_jitter(RTP_session *session);
// Il jitter

unsigned int RTCP_get_total_packet(RTP_session *session);
// Quanti pacchetti ha spedito il server

unsigned int RTCP_get_total_payload_octet(RTP_session *session);
// Quanti byte ha spedito il server

// int RTCP_parse(char *buff,RTP_session *session);
// Effettua il parsing di un pacchetto RTCP

// int RTCP_send_SDES(RTP_session *session);
// Spedisce un pacchetto Source DEScription sulla base dei dati in *session.

// int RTCP_send_SR(RTP_session *session);
// Spedisce un pacchetto Source Report sulla base dei dati in *session.


#endif

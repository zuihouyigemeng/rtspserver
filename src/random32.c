/* * 
 *  $Id: random32.c 20 2004-06-18 17:14:56Z federico $
 *  
 *  This file is part of NeMeSI
 *
 *  NeMeSI -- NEtwork MEdia Streamer I
 *
 *  Copyright (C) 2001 by
 *  	
 *  	Giampaolo "mancho" Mancini - manchoz@inwind.it
 *	Francesco "shawill" Varano - shawill@infinto.it
 *
 *  NeMeSI is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NeMeSI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NeMeSI; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * */

/* From RFC 1889 */

#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fenice/md5global.h>
#include <fenice/md5.h>
#include <fenice/types.h>

#include <string.h>
#include <stdlib.h>

#include "../config.h"

static uint32 md_32(char *string, int length)
{
    /*DEBUG_PRINTF("<><><><><>begin to use md5 to generate ssrc now!\n");*/   /*just for debug,yanf*/
    MD5_CTX context;
    union 
    {
        char c[16];
        uint32 x[4];
    } digest;
    uint32 r;
    int i;

    /*DEBUG_PRINTF("<><><><><>MD5 init now!\n");    */              /*just for debug,yanf*/
    MD5Init(&context);

    /*DEBUG_PRINTF("<><><><><>MD5 updtate now!\n");*/           /*just for debug,yanf*/
    MD5Update(&context, string, length);
    /*DEBUG_PRINTF("<><><><><>MD5 final!\n");*/                        /*just for debug,yanf*/
    MD5Final((unsigned char *)&digest, &context);
    /*DEBUG_PRINTF("<><><><><>end of md5 algorithm\n");*/        /*just for debug,yanf*/         
    r=0;
    for (i=0; i<3; i++)
        r ^= digest.x[i];
    return r;
}

uint32 random32(int type)
{
    struct {
    	int type;
    	struct timeval tv;
    	clock_t cpu;
    	pid_t pid;
    	/*uint32 hid;*/              /*after test,get hostid is a time-consuming work,to improve  speed of dvr,disable it! yanf*/
    	uid_t uid;
    	gid_t gid;
    	struct utsname name;
    } s;

    gettimeofday(&s.tv, NULL);
    uname(&s.name);
    s.type=type;
    s.cpu=clock();
    s.pid=getpid();
    /*s.hid=gethostid();  */    /*after test,get hostid is a time-consuming work,to improve  speed of dvr,disable it! yanf*/
    s.uid=getuid();
    s.gid=getgid();

    return md_32((char *)&s, sizeof(s));  

}

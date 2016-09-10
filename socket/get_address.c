/* * 
 *  $Id: get_address.c 32 2004-07-07 09:50:50Z shawill $
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
#include "../config.h"
#include <fenice/socket.h>
#include <fenice/fnc_log.h>

#define DVRLB_NET_CONF_FILE   "/mnt/mtd/Config/network"

#ifndef WIN32
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/ioctl.h>
#else
	#include <io.h>	
#endif
              
/* Return the IP address of this machine. */
/*DVR LB通过读取文件实现，文件路径为 /mnt/mtd/Config/network*/

char *get_address()
{
  static char     szIp[256];  
  char szDebug[128];

  FILE *fConfig;
  char szLine[128],*cTmp;
  
  if(NULL == (fConfig = fopen(DVRLB_NET_CONF_FILE,"r")))
  {
      sprintf(szDebug,"cannot open network config file:");
      strcat(szDebug,DVRLB_NET_CONF_FILE);
      strcat(szDebug,"\n");
      fnc_log(FNC_LOG_ERR,szDebug);
      DEBUG_PRINTF(szDebug);
      return szIp;
  }

   /*找到主机IP所在的行*/
  while(NULL != (cTmp = fgets(szLine,128,fConfig)))
  {
      if(NULL != (cTmp = strstr(cTmp,"HOSTIP")))
      {
          break;
      }
      else 
      {
          continue;
      }
  }
  
  if(NULL == cTmp)
  {
      sprintf(szDebug,"!!!!!!no hostip line in the file!\n");
      DEBUG_PRINT_ERROR(szDebug);
      fnc_log(FNC_LOG_WARN,szDebug);
      close(fConfig);
      return szIp;
  }

  /*取出IP地址，返回*/
  cTmp = strtok(szLine,"=  ");
  cTmp = strtok(NULL,"=  ");
  strncpy(szIp, cTmp,256);
  
  fclose(fConfig);
  return szIp;
  
  #if 0
  /*DVR LB的gethostbyname无效，必须读文件*/
  char server[256];
  u_char          addr1, addr2, addr3, addr4, temp;
  u_long          InAddr;
  struct hostent *host;

  DEBUG_PRINTF("************beginning to get local host address !************\n");
  iRet = gethostname(server,256);
  if(0 != iRet)
  {
      sprintf(szDebug,"##############gethostname error!ret code is :%d\n",iRet);
      DEBUG_PRINTF(szDebug);
      return Ip;
  }

  sprintf(szDebug,"!!!!!!!!!!!!!!get server name ok!,server name is :%s \n",server);
  DEBUG_PRINTF(szDebug);
  
  host = gethostbyname(server);
  if(NULL == host)
  {
      DEBUG_PRINTF("!!!!!!!!!!!!!@@@@@@@gethostbyname error !\n");
      return Ip;
  }

  DEBUG_PRINTF("************beginning to set address variables !************\n");
  temp = 0;
  InAddr = *(u_int32 *) host->h_addr;
  addr4 = (unsigned char) ((InAddr & 0xFF000000) >> 0x18);
  addr3 = (unsigned char) ((InAddr & 0x00FF0000) >> 0x10);
  addr2 = (unsigned char) ((InAddr & 0x0000FF00) >> 0x8);
  addr1 = (unsigned char) (InAddr & 0x000000FF);

#if (BYTE_ORDER == BIG_ENDIAN)
  temp = addr1;
  addr1 = addr4;
  addr4 = temp;
  temp = addr2;
  addr2 = addr3;
  addr3 = temp;
#endif

  sprintf(Ip, "%d.%d.%d.%d", addr1, addr2, addr3, addr4);

  DEBUG_PRINTF("--------address:");
  DEBUG_PRINTF(Ip);
  DEBUG_PRINTF("---------------\n");
  #endif
}



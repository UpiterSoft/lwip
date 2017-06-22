/*
 * httpd_memory_management.c
 *
 *  Created on: 8 θών. 2017 γ.
 *      Author: andew
 */


#include "lwip/apps/httpd.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tcp.h"

#ifndef HTTP_MEM_RESERVE
#define HTTP_MEM_RESERVE	(4*1024)
#endif

#ifndef HTTP_MEMUSE_LEVEL_1
#define HTTP_MEMUSE_LEVEL_1	(8*1024)
#endif

#ifndef HTTP_MEMUSE_LEVEL_2
#define HTTP_MEMUSE_LEVEL_2	(12*1024)
#endif

extern signed long memused;
extern int last_free_mem;

static u16_t http_bufLimit(int avail_mem) {
	int bufLimit = TCP_MSS;

	if (avail_mem<=0)             {   bufLimit = 64;
	} else if (avail_mem<=1*1024) {   bufLimit = 160;
	} else if (avail_mem<=3*1024) {	  bufLimit = 512;
	} else if (avail_mem<=5*1024) {	  bufLimit = 1024;
	} else if (avail_mem<=6*1024) {	  bufLimit = TCP_MSS;
	}

	if (bufLimit>64){
	  if (memused>HTTP_MEMUSE_LEVEL_1)		  bufLimit /= 2;
	  if (memused>HTTP_MEMUSE_LEVEL_2)		  bufLimit /= 2;
	  if (memused>HTTP_MEMUSE_LEVEL_2+2*1024) bufLimit = 64;
	}

	return bufLimit;
}

u16_t httpGetMaxWriteLen(struct altcp_pcb* pcb) {
	return http_bufLimit(last_free_mem-HTTP_MEM_RESERVE);
}

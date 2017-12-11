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

void memtest();

extern int last_free_mem;


enum  {
	PRIORITY_LOW,
	PRIORITY_HIGH,
};


static uint16_t limit(const uint16_t min, const uint16_t value, const uint16_t max) {
	if (value > max) {
		return max;
	}
	if (value < min) {
		return min;
	}
	return value;
}

static u16_t http_bufLimit(const int avail_mem, const uint8_t priority) {
	enum {
		MINIMUM_BUFFER_SIZE = 64,
		MAXIMUM_BUFFER_SIZE = 2*TCP_MSS,
	};

//	if (avail_mem < 0) {
//		return MINIMUM_BUFFER_SIZE;
//	}
//
//	int bufLimit = limit(MINIMUM_BUFFER_SIZE, avail_mem, MAXIMUM_BUFFER_SIZE);

	int bufLimit = MAXIMUM_BUFFER_SIZE;

	if (avail_mem<=0)           { bufLimit = 64;}
	else if (avail_mem<=1*1024) { bufLimit = 160;}
	else if (avail_mem<=3*1024) { bufLimit = 512;}
	else if (avail_mem<=5*1024) { bufLimit = 1024;}
	else if (avail_mem<=6*1024) { bufLimit = MAXIMUM_BUFFER_SIZE;	memtest();}

	if (bufLimit>64){
	  if (memused>HTTP_MEMUSE_LEVEL_1)		  bufLimit /= 2;
	  if (memused>HTTP_MEMUSE_LEVEL_2)		  bufLimit /= 2;
	  if (memused>HTTP_MEMUSE_LEVEL_2+2*1024) bufLimit = 64;
	}

	if ((priority == PRIORITY_HIGH)  && (avail_mem-256>bufLimit) ) {
		bufLimit = avail_mem/2;
		if (bufLimit > MAXIMUM_BUFFER_SIZE)
			bufLimit = MAXIMUM_BUFFER_SIZE;
	}

//	printf("http_bufLimit: bufLimit = %d, avail_mem = %d\n\r", bufLimit, avail_mem);
	return bufLimit;
}

static struct altcp_pcb * priority_connection = NULL;

uint8_t httpPriorityConnSet(struct altcp_pcb *pcb){
	if (priority_connection == NULL) {
		priority_connection = pcb;
	}
	uint8_t priority =
			(priority_connection == pcb)
			? PRIORITY_HIGH
			: PRIORITY_LOW;

	return priority;
}

void httpPrioriyConnClear(struct altcp_pcb *pcb){
	if (priority_connection == pcb) {
		priority_connection = NULL;
	}
}

u16_t httpGetMaxWriteLen(struct altcp_pcb* pcb) {
	//the often test call the better
	memtest();
	return http_bufLimit(last_free_mem-HTTP_MEM_RESERVE, httpPriorityConnSet(pcb));
}

/**
 * lwIP options file
 */

#include <config.h>
#include <ptrcompat.h>
#include <kernel/assert.h>
#include <osAlloc.h>
#include <stdlib.h>
#include <hardware/mcu.h>

// --- Memory ---
#define MEM_LIBC_MALLOC 1
#define MEMP_MEM_MALLOC 1
//#define mem_free osFree
//#define mem_malloc osAlloc
//#define mem_calloc osCAlloc

#ifdef __cplusplus
extern "C" {
#endif

void* memt_calloc (size_t, size_t);
void* memt_malloc (size_t);
void  memt_free (void*);

#ifdef __cplusplus
}
#endif
// --- Memory ---

extern signed long memused;

#define mem_clib_free memt_free
#define mem_clib_malloc memt_malloc
#define mem_clib_calloc memt_calloc

#define TCP_MSS                         1460
#define TCP_OVERSIZE                    0

#ifdef MCU_GROUP_IS_STM32F1

#define TCP_SND_BUF						(2*TCP_MSS)
// buf size divided by 2
#define HTTP_MEMUSE_LEVEL_1 8*1024
// buf size divided by 4
#define HTTP_MEMUSE_LEVEL_2 12*1024

#define HTTP_MEM_RESERVE 4*1024

#else //MCU_GROUP_IS_STM32F1

#ifdef MCU_GROUP_IS_STM32F2

#define TCP_SND_BUF						(6*TCP_MSS)
// buf size divided by 2
#define HTTP_MEMUSE_LEVEL_1 12*1024
// buf size divided by 4
#define HTTP_MEMUSE_LEVEL_2 16*1024

#define HTTP_MEM_RESERVE 8*1024

#else //MCU_GROUP_IS_STM32F2
#error "MCU is not defined"
#endif //MCU_GROUP_IS_STM32F2

#endif //MCU_GROUP_IS_STM32F1

/*------------------- FS OPTIONS -------------------*/

/** Set this to 1 and provide the functions:
 * - "int fs_open_custom(struct fs_file *file, const char *name)"
 *    Called first for every opened file to allow opening files
 *    that are not included in fsdata(_custom).c
 * - "void fs_close_custom(struct fs_file *file)"
 *    Called to free resources allocated by fs_open_custom().
 */
#define LWIP_HTTPD_CUSTOM_FILES       1

/** LWIP_HTTPD_FS_ASYNC_READ==1: support asynchronous read operations
 * (fs_read_async returns FS_READ_DELAYED and calls a callback when finished).
 */
#define LWIP_HTTPD_FS_ASYNC_READ       1

/** Set this to 1 if you want to include code that creates HTTP headers
 * at runtime. Default is off: HTTP headers are then created statically
 * by the makefsdata tool. Static headers mean smaller code size, but
 * the (readonly) fsdata will grow a bit as every file includes the HTTP
 * header. */
#define LWIP_HTTPD_DYNAMIC_HEADERS 1

/** Maximum retries before the connection is aborted/closed.
 * - number of times pcb->poll is called -> default is 4*500ms = 2s;
 * - reset when pcb->sent is called
 */
#define HTTPD_MAX_RETRIES                   30

/** Priority for tcp pcbs created by HTTPD (very low by default).
 *  Lower priorities get killed first when running out of memory.
 */
#define HTTPD_TCP_PRIO                      TCP_PRIO_MAX

/** Set this to 1 on platforms where strnstr is not available */
#define LWIP_HTTPD_STRNSTR_PRIVATE          1

/** Set this to 0 to drop support for HTTP/0.9 clients (to save some bytes) */
#define LWIP_HTTPD_SUPPORT_V09              0

/** Set this to 1 to enable HTTP/1.1 persistent connections.
 * ATTENTION: If the generated file system includes HTTP headers, these must
 * include the "Connection: keep-alive" header (pass argument "-11" to makefsdata).
 */
#define LWIP_HTTPD_SUPPORT_11_KEEPALIVE     0

#if LWIP_HTTPD_SUPPORT_REQUESTLIST
/** Number of rx pbufs to enqueue to parse an incoming request (up to the first
    newline) */
#define LWIP_HTTPD_REQ_QUEUELEN             20
#endif /* LWIP_HTTPD_SUPPORT_REQUESTLIST */

/** Set this to 0 to not send the SSI tag (default is on, so the tag will
 * be sent in the HTML page */
#define LWIP_HTTPD_SSI_INCLUDE_TAG           0

/* By default, the httpd is limited to send 2*pcb->mss to keep resource usage low
   when http is not an important protocol in the device. */
#define HTTPD_LIMIT_SENDING_TO_2MSS 0

#define HTTP_IS_DATA_VOLATILE(hs) TCP_WRITE_FLAG_COPY

#define HTTP_IS_HDR_VOLATILE(hs, ptr) TCP_WRITE_FLAG_COPY

/* Define this to a function that returns the maximum amount of data to enqueue.*/
#define HTTPD_MAX_WRITE_LEN(pcb) httpGetMaxWriteLen(pcb)

#define LWIP_HOOK_FILENAME <lwip/lwip_hooks.h>

#ifdef SIMULATION
#include "lwipopts_sim.h"
#else
#include <hardware/mcu.h>

/* ---------- Statistics options ---------- */

#define LWIP_STATS              1
#define LWIP_STATS_DISPLAY      0

#if LWIP_STATS
#define LINK_STATS              1
#define IP_STATS                1
#define ICMP_STATS              1
#define IGMP_STATS              0
#define IPFRAG_STATS            0
#define UDP_STATS               1
#define TCP_STATS               1
#define MEM_STATS               0
#define MEMP_STATS              0
#define PBUF_STATS              0
#define SYS_STATS               0
#endif /* LWIP_STATS */

#ifdef CFG_THREADED_LWIP
#define NO_SYS 0
#else
#define NO_SYS 1
#endif

#define SYS_LIGHTWEIGHT_PROT (!NO_SYS)
#define LWIP_NETCONN         (!NO_SYS)
#define LWIP_SOCKET          0

//#ifdef NC30
//#define LWIP_PROVIDE_ERRNO
//#endif
#ifdef CFG_MODULE_NETWORK
#define LWIP_TCP 1
#else
#define LWIP_TCP 0
#endif

#define LWIP_UDP 1
#define LWIP_NETIF_LINK_CALLBACK 1

#define LWIP_NETIF_STATUS_CALLBACK 1

#define LWIP_DNS 1
#define LWIP_DHCP 1
#define LWIP_DHCP_BOOTP_FILE 0

/** Set this to 1 to support SSI (Server-Side-Includes) */
#define LWIP_HTTPD_SSI            0

/** Set this to 1 to support CGI */
#define LWIP_HTTPD_CGI            0
/** Set this to 1 to support HTTP POST */
#define LWIP_HTTPD_SUPPORT_POST   1

/** The server port for HTTPD to use */
#define HTTPD_SERVER_PORT         80

#define MEM_SIZE                        16384
#define PBUF_POOL_SIZE                  8 // with NO_SYS=0, might need to increase this to 32 to avoid deadlocks

/**
 * TCP_LISTEN_BACKLOG: Enable the backlog option for tcp listen pcb.
 */
#define TCP_LISTEN_BACKLOG              1

//#define TCP_SND_QUEUELEN                (2 * (TCP_SND_BUF)/(TCP_MSS))
//#define TCP_WND                         (3 * TCP_MSS)

#define MEMP_NUM_TCP_PCB                1
#define MEMP_NUM_TCP_PCB_LISTEN         1
#define MEMP_NUM_TCP_SEG                8

//#define MEMP_NUM_SYS_TIMEOUT            (4 + LWIP_DNS + LWIP_TCP)

//#define ARP_QUEUEING 0
//#define MEMP_NUM_ARP_QUEUE              10

//#define IP_FRAG 0
//#define IP_FRAG_USES_STATIC_BUF         0

//#define TCP_QUEUE_OOSEQ                 0

// this block is only used if NO_SYS==0
#if NO_SYS == 0
//#define DEFAULT_THREAD_STACKSIZE 0x40
//#define DEFAULT_THREAD_PRIO  30
#endif

//#define DEFAULT_RAW_RECVMBOX_SIZE 20
//#define DEFAULT_UDP_RECVMBOX_SIZE 20
//#define DEFAULT_TCP_RECVMBOX_SIZE 20
//#define DEFAULT_ACCEPTMBOX_SIZE 2

//#define TCPIP_MBOX_SIZE 20 // number of entries
//#define TCPIP_THREAD_STACKSIZE 0x1E0
//#define TCPIP_THREAD_PRIO    130
// ------------------------------------

#define CHECKSUM_GEN_IP                 0
#define CHECKSUM_GEN_UDP                0
#define CHECKSUM_GEN_TCP                0
#define CHECKSUM_CHECK_IP               0
#define CHECKSUM_CHECK_UDP              0
#define CHECKSUM_CHECK_TCP              0

//#define TCP_TMR_INTERVAL       5  /* The TCP timer interval in milliseconds. */

//#define LWIP_DEBUG
//#define MEMP_DEBUG LWIP_DBG_ON
//#define PBUF_DEBUG LWIP_DBG_ON
//#define SYS_DEBUG LWIP_DBG_ON
//#define TCP_INPUT_DEBUG LWIP_DBG_ON
//#define TCP_OUTPUT_DEBUG LWIP_DBG_ON
//#define TCP_DEBUG LWIP_DBG_ON
//#define TIMERS_DEBUG LWIP_DBG_OFF
//#define DHCP_DEBUG LWIP_DBG_ON
//#define TCP_QLEN_DEBUG                  LWIP_DBG_ON
//#define HTTPD_DEBUG         LWIP_DBG_ON
//#define LWIP_STATS_DISPLAY 1


#include <stdio.h>

#define LWIP_RAND _rand

/** Set this to 1 to support fs_read() to dynamically read file data.
 * Without this (default=off), only one-block files are supported,
 * and the contents must be ready after fs_open().
 */
#define LWIP_HTTPD_DYNAMIC_FILE_READ  1

#endif //SIMULATION

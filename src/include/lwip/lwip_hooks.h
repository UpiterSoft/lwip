/*
 * lwip_hooks.h
 *
 *  Created on: 14 θών. 2017 γ.
 *      Author: dmitry
 */

#ifndef SRC_SHARED_LWIP_SUBMODULE_SRC_INCLUDE_LWIP_LWIP_HOOKS_H_
#define SRC_SHARED_LWIP_SUBMODULE_SRC_INCLUDE_LWIP_LWIP_HOOKS_H_


#include <lwip/altcp.h>

u16_t httpGetMaxWriteLen(struct altcp_pcb *pcb);
char * getETagHeader(fs_file_extension * const pextension);
void setCookieSessionID(fs_file_extension * const pextension, const uint32_t session_id);


#endif /* SRC_SHARED_LWIP_SUBMODULE_SRC_INCLUDE_LWIP_LWIP_HOOKS_H_ */

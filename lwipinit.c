#include <lwip/init.h>
#include <lwip/tcpip.h>
#include <lwip/opt.h>
#include <lwip/timeouts.h>

void lwipInit(void)
{
	// initialize TCP/IP
#if NO_SYS
	// single-threaded version
	lwip_init();
#else
	// multi-threaded version
	tcpip_init(NULL, NULL);
#endif
}

void lwipReset(void)
{
#if NO_SYS
	sys_restart_timeouts();
#endif
}

void lwipRun(void)
{
#if NO_SYS
	sys_check_timeouts();
#endif
}

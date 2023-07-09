#ifndef COMMON_H
#define COMMON_H

#include <io/iousb.h>

#ifdef DEVBUILD
#define RA1NPOC_API
#else
#define RA1NPOC_API
#endif

#ifdef DEVBUILD
#define RA1NPOC_STATIC_API
#else
#define RA1NPOC_STATIC_API 
#endif

#ifdef BAKERA1N_MODE

/* BAKERA1N_MODE */
# define RA1NPOC_COMMON_H
# include "../../../include/bakera1n/bakera1n_flag.h"
/* BAKERA1N_MODE */

#else

/* !BAKERA1N_MODE */
# define checkrain_option_none               0x00000000
// KPF options
# define checkrain_option_verbose_boot       (1 << 0)

// Global options
# define checkrain_option_safemode           (1 << 0)
# define checkrain_option_bind_mount         (1 << 1)
# define checkrain_option_overlay            (1 << 2)
# define checkrain_option_force_revert       (1 << 7) /* keep this at 7 */
/* !BAKERA1N_MODE */

#endif

// old KPF options (pongoOS 2.5.x/checkra1n 0.12.4)
#define old_checkrain_option_safemode       (1 << 0)
#define old_checkrain_option_verbose_boot   (1 << 1)

#define kRa1nNone               (0)
#define kRa1nCheckra1nMode      (1 << 0) // based checkra1n v0.12.4b
#define kRa1nPongoLoaderMode    (1 << 1) // based checkra1n v0.1337.0
#define kRa1nPwnDFUMode         (1 << 2) // based gaster
#define kRa1nNewExploitMode     (1 << 3) // use checkra1n v0.1337.0 exploit code
#define kRa1nShowHelpMode       (1 << 4)
#define kRa1nShowListMode       (1 << 5)
#define kRa1nUseCleanDFUMode    (1 << 6)
#define kRa1nShowDebugMode      (1 << 7)

#if defined(RA1NPOC_MODE)
transfer_t USBReqStall(client_t *client);
transfer_t USBReqLeak(client_t *client, unsigned char* blank);
#endif

transfer_t USBReqNoLeak(client_t *client, unsigned char* blank);
transfer_t leak(client_t *client, unsigned char* blank);
transfer_t noLeak(client_t *client, unsigned char* blank);
transfer_t sendData(client_t *client, unsigned char* buf, size_t size);
transfer_t sendDataTO(client_t *client, unsigned char* buf, size_t size, int timeout);
transfer_t getStatus(client_t *client, unsigned char* buf, size_t size);
transfer_t sendAbort(client_t *client);
int sendPongo(client_t *client, const void* pongoBuf, const size_t pongoLen);

#if defined(RA1NPOC_MODE)
const char *IOReturnName(IOReturn res);
int isStalled(IOReturn res);
int isTimeout(IOReturn res);
void preRetry(client_t *client, unsigned int i);
#endif

#endif

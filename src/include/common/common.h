#ifndef COMMON_H
#define COMMON_H

#define DFU_UNKOWN_TYPE (0)
#define DFU_LEGACY      (1 << 1)
#define DFU_IPHONE7     (1 << 2)
#define DFU_IPHONEX     (1 << 3)

int enter_dfu_via_recovery(io_client_t client);
int payload_stage2(io_client_t client, checkra1n_payload_t payload);
int pongo(io_client_t client, checkra1n_payload_t payload);
int connect_to_stage2(io_client_t client, checkra1n_payload_t payload);

transfer_t usb_req_stall(io_client_t client);
transfer_t usb_req_leak(io_client_t client, unsigned char* blank);
transfer_t usb_req_no_leak(io_client_t client, unsigned char* blank);
transfer_t leak(io_client_t client, unsigned char* blank);
transfer_t no_leak(io_client_t client, unsigned char* blank);
transfer_t send_data(io_client_t client, unsigned char* buf, size_t size);
transfer_t send_data_with_time(io_client_t client, unsigned char* buf, size_t size, int timeout);
transfer_t get_status(io_client_t client, unsigned char* buf, size_t size);
transfer_t send_abort(io_client_t client);

transfer_t usb_req_leak_with_async(io_client_t client,
                                   unsigned char* blank,
                                   int usleep_time,
                                   async_transfer_t transfer);

/*
 * pongoOS conf
 *
 */
#ifdef BUILTIN_PAYLOAD
#define BOOTARGS_STR_PTR    (0x15801E)
#define KPF_FLAGS_PTR       (0x158018)
#endif // BUILTIN_PAYLOAD

/*
 * Copyright (C) 2019-2021 checkra1n team
 * This file is part of pongoOS.
 */
#define DEFAULT_BOOTARGS "rootdev=md0"
#define MAX_BOOTARGS_LEN 256
typedef enum {
    checkrain_option_none               = 0,
    checkrain_option_all                = -1,
    checkrain_option_failure            = -2,
    
    checkrain_option_safemode           = 1 << 0,
    checkrain_option_verbose_boot       = 1 << 1,
    checkrain_option_verbose_logging    = 1 << 2,
    checkrain_option_demote             = 1 << 3,
    checkrain_option_pongo_shell        = 1 << 4,
    checkrain_option_early_exit         = 1 << 5,
} checkrain_option_t, *checkrain_option_p;

#define checkrain_set_option(options, option, enabled) do { \
if (enabled)                                            \
options = (checkrain_option_t)(options | option);   \
else                                                    \
options = (checkrain_option_t)(options & ~option);  \
} while (0);

#endif

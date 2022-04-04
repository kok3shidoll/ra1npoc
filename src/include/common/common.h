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

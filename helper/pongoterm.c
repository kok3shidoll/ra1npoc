/*
 * ra1npoc - helper/pongoterm.c
 *
 * Copyright (c) 2021 - 2023 kok3shidoll
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <io/iousb.h>
#include <common/log.h>
#include <common/common.h>
#include <pongoterm.h>

enum AUTOBOOT_STAGE CURRENT_STAGE = NONE;

extern bool use_safemode;
extern bool use_verbose_boot;
extern char* bootArgs;

#if defined(BAKERA1N_MODE)
extern bool disable_cfprefsd_hook;
extern bool use_lightweight_overlay;
#endif

static uint32_t kpf_flags = checkrain_option_none;
static uint32_t checkra1n_flags = checkrain_option_none;

#if defined(BAKERA1N_MODE)
# include "../headers/kpf.h"
# include "../headers/ramdisk.h"
# include "../headers/overlay.h"
# include "../headers/overlay_lite.h"
#endif

#if defined(RA1NPOC_MODE)
# include "../headers/legacy_kpf.h"
# include "../headers/legacy_ramdisk.h"
RA1NPOC_API int oldPongoTerm(client_t *client)
{
    transfer_t result;
    
    while(1)
    {
        char buf[0x2000] = {};
        uint32_t outpos = 0;
        uint8_t in_progress = 1;
        while(in_progress)
        {
            result = IOUSBControlTransfer(client, 0xa1, 2, 0, 0, (unsigned char *)&in_progress, (uint32_t)sizeof(in_progress));
            if(result.ret == kIOReturnSuccess)
            {
                result = IOUSBControlTransfer(client, 0xa1, 1, 0, 0, (unsigned char *)(buf + outpos), 0x1000);
                if(result.ret == kIOReturnSuccess)
                {
                    outpos += result.wLenDone;
                    if(outpos > 0x1000)
                    {
                        memmove(buf, buf + outpos - 0x1000, 0x1000);
                        outpos = 0x1000;
                    }
                }
            }
            if(result.ret != kIOReturnSuccess)
            {
                goto bad;
            }
        }
        
        {
            result = IOUSBControlTransfer(client, 0x21, 4, 0xffff, 0, NULL, 0);
            if(result.ret != kIOReturnSuccess)
            {
                goto bad;
            }
        }
        
        if(CURRENT_STAGE == NONE)
        {
            CURRENT_STAGE = SETUP_STAGE_FUSE;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_FUSE)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"fuse lock\n", (uint32_t)(strlen("fuse lock\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "fuse lock");
                CURRENT_STAGE = SETUP_STAGE_SEP;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_SEP)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"sep auto\n", (uint32_t)(strlen("sep auto\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "sep auto");
                CURRENT_STAGE = SEND_STAGE_KPF;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SEND_STAGE_KPF)
        {
            size_t size = legacy_kpf_len;
            result = IOUSBControlTransfer(client, 0x21, 1, 0, 0, (unsigned char *)&size, 4);
            if(result.ret == kIOReturnSuccess)
            {
                result = IOUSBBulkUpload(client, legacy_kpf, legacy_kpf_len);
                if(result.ret == kIOReturnSuccess)
                {
                    LOG("sended file: %s: %llu bytes", "kpf", (unsigned long long)legacy_kpf_len);
                    CURRENT_STAGE = SETUP_STAGE_KPF;
                }
                else
                {
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                }
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_KPF)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"modload\n", (uint32_t)(strlen("modload\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "modload");
                CURRENT_STAGE = SEND_STAGE_RAMDISK;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SEND_STAGE_RAMDISK)
        {
            size_t size = legacy_ramdisk_len;
            result = IOUSBControlTransfer(client, 0x21, 1, 0, 0, (unsigned char *)&size, 4);
            if(result.ret == kIOReturnSuccess)
            {
                result = IOUSBBulkUpload(client, legacy_ramdisk, legacy_ramdisk_len);
                if(result.ret == kIOReturnSuccess)
                {
                    LOG("sended file: %s: %llu bytes", "ramdisk", (unsigned long long)legacy_ramdisk_len);
                    CURRENT_STAGE = SETUP_STAGE_RAMDISK;
                }
                else
                {
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                }
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_RAMDISK)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"ramdisk\n", (uint32_t)(strlen("ramdisk\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "ramdisk");
                CURRENT_STAGE = SETUP_STAGE_KPF_FLAGS;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_KPF_FLAGS)
        {
            
            if(use_verbose_boot)
            {
                kpf_flags |= old_checkrain_option_verbose_boot;
            }
            
            if(use_safemode)
            {
                checkra1n_flags |= old_checkrain_option_safemode;
            }
            
            char str[64];
            memset(&str, 0x0, 64);
            sprintf(str, "kpf_flags 0x%08x\n", kpf_flags);
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)str, (uint32_t)(strlen(str)));
            if(result.ret == kIOReturnSuccess)
            {
                memset(&str, 0x0, 64);
                sprintf(str, "kpf_flags 0x%08x", kpf_flags);
                LOG("kpf_flags: 0x%08x", kpf_flags);
                
                CURRENT_STAGE = SETUP_STAGE_XARGS;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_XARGS)
        {
            char str[256];
            memset(&str, 0x0, 256);
            
            char* defaultBootArgs = NULL;
            
            if(!use_verbose_boot)
            {
                defaultBootArgs = "rootdev=md0";
            }
            
            if(use_verbose_boot)
            {
                defaultBootArgs = "-v rootdev=md0";
            }
            
            if(defaultBootArgs)
            {
                if(strlen(defaultBootArgs) > 256)
                {
                    ERR("defaultBootArgs is too large!");
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                    continue;
                }
                sprintf(str, "%s", defaultBootArgs);
            }
            
            if(bootArgs)
            {
                if((strlen(str) + strlen(bootArgs)) > 256)
                {
                    ERR("bootArgs is too large!");
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                    continue;
                }
                sprintf(str, "%s %s", str, bootArgs);
            }
            
            char xstr[256 + 7];
            memset(&xstr, 0x0, 256 + 7);
            sprintf(xstr, "xargs %s\n", str);
            
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)xstr, (uint32_t)(strlen(xstr)));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("xargs: %s", str);
                CURRENT_STAGE = BOOTUP_STAGE;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == BOOTUP_STAGE)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"bootx\n", (uint32_t)(strlen("bootx\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "bootx");
                return 0;
            }
            else
            {
                LOG("maybe sended cmd: %s", "bootx");
                return 0;
            }
            continue;
        }
        
        if(CURRENT_STAGE == USB_TRANSFER_ERROR)
        {
        bad:
            ERR("WTF?!");
            if(client)
            {
                IOUSBClose(client);
            }
            return -1;
        }
    }
    return 0;
}
#endif

#if defined(BAKERA1N_MODE)
RA1NPOC_API int pongoTerm(client_t *client)
{
    transfer_t result;
    
    while(1)
    {
        char buf[0x2000] = {};
        uint32_t outpos = 0;
        uint8_t in_progress = 1;
        while(in_progress)
        {
            result = IOUSBControlTransfer(client, 0xa1, 2, 0, 0, (unsigned char *)&in_progress, (uint32_t)sizeof(in_progress));
            if(result.ret == kIOReturnSuccess)
            {
                result = IOUSBControlTransfer(client, 0xa1, 1, 0, 0, (unsigned char *)(buf + outpos), 0x1000);
                if(result.ret == kIOReturnSuccess)
                {
                    outpos += result.wLenDone;
                    if(outpos > 0x1000)
                    {
                        memmove(buf, buf + outpos - 0x1000, 0x1000);
                        outpos = 0x1000;
                    }
                }
            }
            if(result.ret != kIOReturnSuccess)
            {
                goto bad;
            }
        }
        
        {
            result = IOUSBControlTransfer(client, 0x21, 4, 0xffff, 0, NULL, 0);
            if(result.ret != kIOReturnSuccess)
            {
                goto bad;
            }
        }
        
        if(CURRENT_STAGE == NONE)
        {
            CURRENT_STAGE = SETUP_STAGE_FUSE;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_FUSE)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"fuse lock\n", (uint32_t)(strlen("fuse lock\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "fuse lock");
                CURRENT_STAGE = SETUP_STAGE_SEP;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_SEP)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"sep auto\n", (uint32_t)(strlen("sep auto\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "sep auto");
                CURRENT_STAGE = SEND_STAGE_KPF;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SEND_STAGE_KPF)
        {
            size_t size = kpf_len;
            result = IOUSBControlTransfer(client, 0x21, 1, 0, 0, (unsigned char *)&size, 4);
            if(result.ret == kIOReturnSuccess)
            {
                result = IOUSBBulkUpload(client, kpf, kpf_len);
                if(result.ret == kIOReturnSuccess)
                {
                    LOG("sended file: %s: %llu bytes", "kpf", (unsigned long long)kpf_len);
                    CURRENT_STAGE = SETUP_STAGE_KPF;
                }
                else
                {
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                }
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_KPF)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"modload\n", (uint32_t)(strlen("modload\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "modload");
                CURRENT_STAGE = SEND_STAGE_RAMDISK;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SEND_STAGE_RAMDISK)
        {
            size_t size = ramdisk_dmg_len;
            result = IOUSBControlTransfer(client, 0x21, 1, 0, 0, (unsigned char *)&size, 4);
            if(result.ret == kIOReturnSuccess)
            {
                result = IOUSBBulkUpload(client, ramdisk_dmg, ramdisk_dmg_len);
                if(result.ret == kIOReturnSuccess)
                {
                    LOG("sended file: %s: %llu bytes", "ramdisk", (unsigned long long)ramdisk_dmg_len);
                    CURRENT_STAGE = SETUP_STAGE_RAMDISK;
                }
                else
                {
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                }
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_RAMDISK)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"ramdisk\n", (uint32_t)(strlen("ramdisk\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "ramdisk");
                CURRENT_STAGE = SEND_STAGE_OVERLAY;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SEND_STAGE_OVERLAY)
        {
            if(use_lightweight_overlay == false)
            {
                size_t size = overlay_dmg_len;
                result = IOUSBControlTransfer(client, 0x21, 1, 0, 0, (unsigned char *)&size, 4);
                if(result.ret == kIOReturnSuccess)
                {
                    result = IOUSBBulkUpload(client, overlay_dmg, overlay_dmg_len);
                    if(result.ret == kIOReturnSuccess)
                    {
                        LOG("sended file: %s: %llu bytes", "overlay", (unsigned long long)overlay_dmg_len);
                        CURRENT_STAGE = SETUP_STAGE_OVERLAY;
                    }
                    else
                    {
                        CURRENT_STAGE = USB_TRANSFER_ERROR;
                    }
                }
            }
            else
            {
                // lightweight overlay
                size_t size = overlay_lite_dmg_len;
                result = IOUSBControlTransfer(client, 0x21, 1, 0, 0, (unsigned char *)&size, 4);
                if(result.ret == kIOReturnSuccess)
                {
                    result = IOUSBBulkUpload(client, overlay_lite_dmg, overlay_lite_dmg_len);
                    if(result.ret == kIOReturnSuccess)
                    {
                        LOG("sended file: %s: %llu bytes", "overlay", (unsigned long long)overlay_lite_dmg_len);
                        CURRENT_STAGE = SETUP_STAGE_OVERLAY;
                    }
                    else
                    {
                        CURRENT_STAGE = USB_TRANSFER_ERROR;
                    }
                }
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_OVERLAY)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"overlay\n", (uint32_t)(strlen("overlay\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "overlay");
                CURRENT_STAGE = SETUP_STAGE_KPF_FLAGS;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_KPF_FLAGS)
        {
            
            if(use_verbose_boot)
            {
                kpf_flags |= checkrain_option_verbose_boot;
            }
            
            char str[64];
            memset(&str, 0x0, 64);
            sprintf(str, "kpf_flags 0x%08x\n", kpf_flags);
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)str, (uint32_t)(strlen(str)));
            if(result.ret == kIOReturnSuccess)
            {
                memset(&str, 0x0, 64);
                sprintf(str, "kpf_flags 0x%08x", kpf_flags);
                LOG("kpf_flags: 0x%08x", kpf_flags);
                CURRENT_STAGE = SETUP_STAGE_CHECKRAIN_FLAGS;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_CHECKRAIN_FLAGS)
        {
            
            if(use_safemode)
            {
                checkra1n_flags |= checkrain_option_safemode;
            }
            
            if(disable_cfprefsd_hook)
            {
                checkra1n_flags |= checkrain_option_no_cfprefsd_hook;
            }
            
            char str[64];
            memset(&str, 0x0, 64);
            sprintf(str, "checkra1n_flags 0x%08x\n", checkra1n_flags);
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)str, (uint32_t)(strlen(str)));
            if(result.ret == kIOReturnSuccess)
            {
                memset(&str, 0x0, 64);
                sprintf(str, "checkra1n_flags 0x%08x", checkra1n_flags);
                LOG("checkra1n_flags: 0x%08x", checkra1n_flags);
                CURRENT_STAGE = SETUP_STAGE_XARGS;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == SETUP_STAGE_XARGS)
        {
            char str[256];
            memset(&str, 0x0, 256);
            
            char* defaultBootArgs = NULL;
            
            if(!use_verbose_boot)
            {
                defaultBootArgs = "rootdev=md0";
            }
            
            if(use_verbose_boot)
            {
                defaultBootArgs = "-v rootdev=md0";
            }
            
            if(defaultBootArgs)
            {
                if(strlen(defaultBootArgs) > 256)
                {
                    ERR("defaultBootArgs is too large!");
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                    continue;
                }
                sprintf(str, "%s", defaultBootArgs);
            }
            
            if(bootArgs)
            {
                // sprintf(str, "xargs %s\n", bootArgs);
                if((strlen(str) + strlen(bootArgs)) > 256)
                {
                    ERR("bootArgs is too large!");
                    CURRENT_STAGE = USB_TRANSFER_ERROR;
                    continue;
                }
                sprintf(str, "%s %s", str, bootArgs);
            }
            
            
            char xstr[256 + 7];
            memset(&xstr, 0x0, 256 + 7);
            sprintf(xstr, "xargs %s\n", str);
            
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)xstr, (uint32_t)(strlen(xstr)));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("xargs: %s", str);
                CURRENT_STAGE = BOOTUP_STAGE;
            }
            else
            {
                CURRENT_STAGE = USB_TRANSFER_ERROR;
            }
            continue;
        }
        
        if(CURRENT_STAGE == BOOTUP_STAGE)
        {
            result = IOUSBControlTransfer(client, 0x21, 3, 0, 0, (unsigned char *)"bootx\n", (uint32_t)(strlen("bootx\n")));
            if(result.ret == kIOReturnSuccess)
            {
                LOG("sended cmd: %s", "bootx");
                return 0;
            }
            else
            {
                // CURRENT_STAGE = USB_TRANSFER_ERROR;
                LOG("maybe sended cmd: %s", "bootx");
                return 0;
            }
            continue;
        }
        
        if(CURRENT_STAGE == USB_TRANSFER_ERROR)
        {
        bad:
            ERR("WTF?!");
            if(client)
            {
                IOUSBClose(client);
            }
            return -1;
        }
    }
    return 0;
}
#endif

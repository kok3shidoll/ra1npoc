/*
 * ra1npoc - common.c
 *
 * Copyright (c) 2021 dora2ios
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
#include <common/common.h>
#include <time.h>

struct checkra1n_device_list {
    uint16_t        cpid;
    uint8_t         bdid;
    int     enterdfuType;
};

static struct checkra1n_device_list devlists[] = {
    // Apple A7
    { 0x8960, 0xff, DFU_LEGACY }, // all have legacy type
    // Apple A8
    { 0x7000, 0xff, DFU_LEGACY }, // all have legacy type
    // Apple A8X
    { 0x7001, 0xff, DFU_LEGACY }, // all have legacy type
    // Apple A9(S)
    { 0x8000, 0xff, DFU_LEGACY }, // all have legacy type
    // Apple A9(T)
    { 0x8003, 0xff, DFU_LEGACY }, // all have legacy type
    // Apple A9X
    { 0x8001, 0xff, DFU_LEGACY },
    // Apple A10 Fusion
    { 0x8010, 0x08, DFU_IPHONE7 }, // iPhone 7
    { 0x8010, 0x0c, DFU_IPHONE7 }, // iPhone 7
    { 0x8010, 0x0a, DFU_IPHONE7 }, // iPhone 7 Plus
    { 0x8010, 0x0e, DFU_IPHONE7 }, // iPhone 7 Plus
    { 0x8010, 0x16, DFU_IPHONE7 }, // iPod touch 7
    { 0x8010, 0x18, DFU_LEGACY  }, // iPad 6
    { 0x8010, 0x1a, DFU_LEGACY  }, // iPad 6
    { 0x8010, 0x1c, DFU_LEGACY  }, // iPad 7
    { 0x8010, 0x1e, DFU_LEGACY  }, // iPad 7
    // Apple A10X Fusion
    { 0x8011, 0xff, DFU_LEGACY }, // all have legacy type
    // Apple T2
    { 0x8012, 0xff, DFU_UNKOWN_TYPE }, // i don't know :/
    // Apple A11 Bionic
    { 0x8015, 0xff, DFU_IPHONEX }, // iPhone 8/X
    // err
    { 0xffff, 0xff, DFU_UNKOWN_TYPE }
};

#if defined(KPF_FLAGS_PTR) && defined(BOOTARGS_STR_PTR)
extern int8_t kpf_flags;
extern const char* bootargs;
#endif

long cpuTime;

static void interval(double sec)
{
    long now;
    double n_sec;
    double b_sec = (double)cpuTime / CLOCKS_PER_SEC;
    while(1) {
        now = clock();
        n_sec = (double)now / CLOCKS_PER_SEC;
        if ((n_sec-b_sec) > sec)
            break;
    }
    cpuTime = now;
}

static void prog(int sec)
{
    int i=0;
    int j=0;
    
    for(i=0; i<sec; i++) {
        printf("[");
        for (j=0;j<i+1;j++)
            printf("=");
        for (;j<sec;j++)
            printf(" ");
        printf("] (%d/%d sec)\r", i+1, sec);
        fflush(stdout);
        interval(1);
    }
    printf("\n");
}

int enter_dfu_via_recovery(io_client_t client)
{
    LOG("[%s] Waiting for device in Recovery mode...", __FUNCTION__);
    while(1) {
        if(client) {
            io_close(client);
            client = NULL;
        }
        io_open(&client, DEVICE_RECOVERY_MODE_2, true);
        if(client) break;
    }
    if(!client) {
        ERROR("[%s] ERROR: Failed to find the device in Recovery mode", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] CONNECTED", __FUNCTION__);
    
    if(client->hasSerialStr == false){
        read_serial_number(client);
    }
    
    DEBUGLOG("[%s] CPID: 0x%02x, BDID: 0x%01x", __FUNCTION__, client->devinfo.cpid, client->devinfo.bdid);
    
    int enterdfuType = DFU_UNKOWN_TYPE;
    for (int i=0; devlists[i].cpid != 0xffff; i++) {
        if (client->devinfo.cpid == devlists[i].cpid) {
            // check t8010: multiple types are mixed :/
            if (client->devinfo.cpid == 0x8010) {
                for(int j=i; devlists[j].bdid != 0xff; j++) {
                    if (client->devinfo.bdid == devlists[j].bdid) {
                        enterdfuType = devlists[i].enterdfuType;
                        break;
                    }
                }
            } else {
                enterdfuType = devlists[i].enterdfuType;
                break;
            }
        }
    }
    DEBUGLOG("[%s] enterdfuType: %x", __FUNCTION__, enterdfuType);
    
    const char* btn = NULL;
    int step2_sec = 4;
    int step3_sec = 10;
    
    if (enterdfuType == DFU_LEGACY)
    { /* DFU_LEGACY */
        btn = "Home button";
    }
    else if (enterdfuType == DFU_IPHONE7 ||
             enterdfuType == DFU_IPHONEX)
    { /* DFU_IPHONE7, DFU_IPHONEX */
        btn = "Volume down button";
    }
    else
    { /* DFU_UNKOWN_TYPE */
        ERROR("[%s] This mode is not yet supported for this device.", __FUNCTION__);
        return -1;
    } /* DFU_UNKOWN_TYPE */
    
    printf("\n");
    
    printf("================\n");
    printf("::\n");
    printf(":: %s\n", __FUNCTION__);
    printf(":: CPID: 0x%02x BDID: 0x%02x TYPE: 0x%02x\n", client->devinfo.cpid, client->devinfo.bdid, enterdfuType);
    printf("::\n");
    printf(":: \x1b[34mTime to put the device into DFU mode\x1b[39m\n");
    printf(":: \x1b[34mPlease follow the instructions below to operate the device.\x1b[39m\n");
    printf("::\n");
    printf(":: \x1b[32mSTEP1: Press \x1b[31m<enter>\x1b[32m key.\x1b[39m\n");
    printf(":: \x1b[32mSTEP2: Press and hold the Side and %ss together \x1b[31m(%dsec)\x1b[39m\n", btn, step2_sec);
    printf(":: \x1b[32mSTEP3: Release the Side button But keep holding the %s \x1b[31m(%dsec)\x1b[39m\n", btn, step3_sec);
    printf("================\n");
    printf("\n");
    printf("\x1b[34mready?\x1b[39m\n");
    printf("\x1b[32m[STEP1] Press <enter> key\x1b[39m >> ");
    getchar();
    
    printf("\n");
    LOG("[STEP2] Press and hold the Side and %s together (%dsec)", btn, step2_sec);
    int j=0;
    for(int i=0; i<step2_sec; i++) {
        if(i==1)
            send_reboot_via_recovery(client);
        printf("[");
        for (j=0;j<i+1;j++)
            printf("=");
        for (;j<step2_sec;j++)
            printf(" ");
        printf("] (%d/%d sec)\r", i+1, step2_sec);
        fflush(stdout);
        interval(1);
    }
    printf("\n");
    
    LOG("[STEP3] Release the Side button But keep holding the %s (%dsec)", btn, step3_sec);
    prog(step3_sec);
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 5, DEVICE_DFU, USB_NO_RESET, false, 10000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to put the device into DFU mode", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] DONE!", __FUNCTION__);
    io_close(client);
    client = NULL;
    return 0;
}

int payload_stage2(io_client_t client, checkra1n_payload_t payload)
{
    transfer_t result;
    
    {
        size_t len = 0;
        size_t size;
        while(len < payload.stage2_len) {
            size = ((payload.stage2_len - len) > 0x800) ? 0x800 : (payload.stage2_len - len);
            result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, (unsigned char*)&payload.stage2[len], size);
            if(result.wLenDone != size || result.ret != kIOReturnSuccess){
                ERROR("[%s] ERROR: Failed to send stage2 [%x, %x]", __FUNCTION__, result.ret, (unsigned int)result.wLenDone);
                return -1;
            }
            len += size;
        }
    }
    DEBUGLOG("[%s] (1/2) %x", __FUNCTION__, result.ret);
    usleep(1000);
    
    result = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    DEBUGLOG("[%s] (2/2) %x", __FUNCTION__, result.ret);
    usleep(1000);
    
    return 0;
}

int pongo(io_client_t client, checkra1n_payload_t payload)
{
    transfer_t result;
    
    unsigned char blank[8];
    memset(&blank, '\0', 8);
    
    result = usb_ctrl_transfer(client, 0x40, 64, 0x03e8, 0x01f4, NULL, 0); // ?
    DEBUGLOG("[%s] (1/6) %x", __FUNCTION__, result.ret);
    
#if defined(KPF_FLAGS_PTR) && defined(BOOTARGS_STR_PTR)
    const char* newArgs = bootargs; // set xnu boot-arg commandline
    size_t newArgsLen = strlen(newArgs) + 1;
    size_t newArgBufLen = (newArgsLen + 3) / 4 * 4;
    char bootArgsBuf[newArgBufLen];
    strlcpy(bootArgsBuf, newArgs, newArgBufLen);
    memset(bootArgsBuf + newArgsLen, 0, newArgBufLen - newArgsLen);
    memcpy(payload.pongoOS+BOOTARGS_STR_PTR, bootArgsBuf, newArgBufLen);
    
    *(int8_t*)(payload.pongoOS+KPF_FLAGS_PTR) = kpf_flags;
#endif
    
    {
        size_t len = 0;
        size_t size;
        while(len < payload.pongoOS_len) {
            size = ((payload.pongoOS_len - len) > 0x800) ? 0x800 : (payload.pongoOS_len - len);
            result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, (unsigned char*)&payload.pongoOS[len], size);
            if(result.wLenDone != size || result.ret != kIOReturnSuccess){
                ERROR("[%s] ERROR: Failed to send pongoOS [%x, %x]", __FUNCTION__, result.ret, (unsigned int)result.wLenDone);
                return -1;
            }
            len += size;
        }
    }
    
    DEBUGLOG("[%s] (2/6) %x", __FUNCTION__, result.ret);
    usleep(10000);
    
    result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, NULL, 0);
    DEBUGLOG("[%s] (3/6) %x", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer(client, 0xa1, 3, 0x0000, 0x0000, blank, 8);
    DEBUGLOG("[%s] (4/6) %x", __FUNCTION__, result.ret);

    result = usb_ctrl_transfer(client, 0xa1, 3, 0x0000, 0x0000, blank, 8);
    DEBUGLOG("[%s] (5/6) %x", __FUNCTION__, result.ret);

    result = usb_ctrl_transfer(client, 0xa1, 3, 0x0000, 0x0000, blank, 8);
    DEBUGLOG("[%s] (6/6) %x", __FUNCTION__, result.ret);
    usleep(10000);
    
    return 0;
}

int connect_to_stage2(io_client_t client, checkra1n_payload_t payload)
{
    int r;
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 15, DEVICE_STAGE2, USB_RESET|USB_REENUMERATE, false, 5000000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to connect to checkra1n DFU", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] connected to Stage2", __FUNCTION__);
    usleep(10000);
    
    LOG("[%s] sending pongoOS", __FUNCTION__);
    r = pongo(client, payload);
    if(r != 0){
        return -1;
    }
    
    io_reset(client, USB_RESET|USB_REENUMERATE);
    io_close(client);
    
    LOG("[%s] BOOTED", __FUNCTION__);
    
    return 0;
}

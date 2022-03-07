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

#if defined(KPF_FLAGS_PTR) && defined(BOOTARGS_STR_PTR)
extern int8_t kpf_flags;
extern const char* bootargs;
#endif

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

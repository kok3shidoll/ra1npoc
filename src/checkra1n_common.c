#include <iousb.h>
#include <checkra1n_common.h>

int payload_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload)
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

int pongo(io_client_t client, uint16_t cpid, checkra1n_payload_t payload)
{
    transfer_t result;
    
    unsigned char blank[8];
    memset(&blank, '\0', 8);
    
    result = usb_ctrl_transfer(client, 0x40, 64, 0x03e8, 0x01f4, NULL, 0); // ?
    DEBUGLOG("[%s] (1/6) %x", __FUNCTION__, result.ret);
    
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

int connect_to_stage2(io_client_t client, uint16_t cpid, checkra1n_payload_t payload)
{
    int r;
    IOReturn result;
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    result = io_reset(client);
    //if(result != kIOReturnSuccess){
    //    ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
    //    io_close(client);
    //    client = NULL;
    //    return -1;
    //}
    io_close(client);
    client = NULL;
    sleep(5);
    get_device_time_stage(&client, 15, DEVICE_STAGE2, false);
    if(!client) {
        ERROR("[%s] ERROR: Failed to connect to checkra1n DFU", __FUNCTION__);
        return -1;
    }
    
    LOG_DONE("[%s] connected to Stage2", __FUNCTION__);
    usleep(10000);
    
    LOG_PROGRESS("[%s] sending pongoOS", __FUNCTION__);
    r = pongo(client, cpid, payload);
    if(r != 0){
        return -1;
    }
    
    result = io_reset(client);
    //if(result != kIOReturnSuccess){
    //    ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
    //    io_close(client);
    //    client = NULL;
    //    return -1;
    //}
    io_close(client);
    
    LOG_DONE("[%s] BOOTED", __FUNCTION__);
    return 0;
}

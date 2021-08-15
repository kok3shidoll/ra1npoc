#include <iousb.h>

#include <checkra1n_common.h>
#include <checkra1n_t8010_t8015.h>
#include <checkra1n_s8000.h>
#include <checkra1n_s5l8960x.h>

static int open_file(char *file, unsigned int *sz, void **buf){
    FILE *fd = fopen(file, "r");
    if (!fd) {
        printf("error opening %s\n", file);
        return -1;
    }
    
    fseek(fd, 0, SEEK_END);
    *sz = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    *buf = malloc(*sz);
    if (!*buf) {
        printf("error allocating file buffer\n");
        fclose(fd);
        return -1;
    }
    
    fread(*buf, *sz, 1, fd);
    fclose(fd);
    
    return 0;
}

io_client_t client;
checkra1n_payload_t payload;

static void log_dev(){
    LOG_WAIT("* checkRAIN clone for iOS (by interception)");
    LOG_DONE("** made by @dora2ios");
}

static void usage(char** argv) {
    printf("Usage: %s [option] over1 over2 stage2 payload\n", argv[0]);
    //printf("\t-p [flag]\tPut device in pwned DFU mode\n");
    printf("\t--a7   \x1b[36ms5l8960x\x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
    printf("\t--a9   \x1b[36ms8000   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
    printf("\t--a10  \x1b[36mt8010   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
    printf("\t--a11  \x1b[36mt8015   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
    printf("\n");
}

int main(int argc, char** argv){
    
    uint16_t devmode;
    if(argc != 6) {
        usage(argv);
        return -1;
    }
    
    char *over1_path = argv[2];
    char *over2_path = argv[3];
    char *stage2_path = argv[4];
    char *pongoOS_path = argv[5];
    
    memset(&payload, '\0', sizeof(checkra1n_payload_t));
    
    if(open_file(over1_path, &payload.over1_len, &payload.over1) != 0) return -1;
    if(open_file(over2_path, &payload.over2_len, &payload.over2) != 0) return -1;
    if(open_file(stage2_path, &payload.stage2_len, &payload.stage2) != 0) return -1;
    if(open_file(pongoOS_path, &payload.pongoOS_len, &payload.pongoOS) != 0) return -1;
    
#ifdef HAVE_DEBUG2
    printf("kIOReturnSuccess: %x\n", kIOReturnSuccess);
    printf("kIOUSBPipeStalled: %x\n", kIOUSBPipeStalled);
    printf("kUSBHostReturnPipeStalled: %x\n", kUSBHostReturnPipeStalled);
    printf("kIOReturnTimeout: %x\n", kIOReturnTimeout);
    printf("kIOUSBTransactionTimeout: %x\n", kIOUSBTransactionTimeout);
    printf("kIOReturnNotResponding: %x\n", kIOReturnNotResponding);
#endif
    
    if(!strcmp(argv[1], "--a10")) {
        devmode = 0x8010;
    } else if(!strcmp(argv[1], "--a11")) {
        devmode = 0x8015;
    } else if(!strcmp(argv[1], "--a9")) {
        devmode = 0x8000;
    } else if(!strcmp(argv[1], "--a7")) {
        devmode = 0x8960;
    } else {
        usage(argv);
        return -1;
    }
    
    log_dev();
    
    LOG_WAIT("[%s] Waiting for device in DFU mode...", __FUNCTION__);
    
    // For iOS 10 and lower:
    //    This device cannot connect to Stage2 because lack of power supply when using lightning to USB camera adapter.
    //    This device can be connected by powering the lightning to USB 3 camera adapter.
    
    // If the device is in Stage2, send pongoOS.
    if(get_device(DEVICE_STAGE2, true) == 0){
        LOG_DONE("[%s] CONNECTED: STAGE2", __FUNCTION__);
        if(devmode == 0x8010 || devmode == 0x8015 || devmode == 0x8000 || devmode == 0x8960){
            connect_to_stage2(client, devmode, payload);
        }
        return 0;
    }
    
    while(get_device(DEVICE_DFU, true) != 0) {
        sleep(1);
    }
    LOG_DONE("[%s] CONNECTED", __FUNCTION__);
    
    if(client->hasSN == false){
        read_serial_number(client); // For iOS 10 and lower
    }
    
    LOG_DONE("[%s] CPID: 0x%02x, STRG: [%s]", __FUNCTION__, client->devinfo.cpid, client->devinfo.srtg);
    
    if(client->hasSN != true){
        LOG_ERROR("[%s] Serial number was not found!", __FUNCTION__);
        return -1;
    }
    
    if((client->devinfo.cpid == 0x8010)&&(devmode == 0x8010)){
        checkra1n_t8010_t8015(client, client->devinfo.cpid, payload); // A10 Fusion
    } else if((client->devinfo.cpid == 0x8015)&&(devmode == 0x8015)){
        checkra1n_t8010_t8015(client, client->devinfo.cpid, payload); // A11 Bionic
    } else if((client->devinfo.cpid == 0x8000)&&(devmode == 0x8000)){
        checkra1n_s8000(client, client->devinfo.cpid, payload);       // A9
    } else if((client->devinfo.cpid == 0x8960)&&(devmode == 0x8960)){
        checkra1n_s5l8960x(client, client->devinfo.cpid, payload);    // A7
    }
    
    return 0;
}


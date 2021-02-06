#include <iousb.h>
#include <checkra1n_t8010_t8015.h>

int open_file(char *file, unsigned int *sz, void **buf){
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

void log_dev(){
    LOG_WAIT("* checkRAIN clone for iOS (by interception)");
    LOG_DONE("*** made by @dora2ios");
}

static void usage(char** argv) {
    printf("Usage: %s [option] over1 over2 stage2 payload\n", argv[0]);
    //printf("\t-p [flag]\tPut device in pwned DFU mode\n");
    printf("\t--a10  \x1b[36mt8010\x1b[39m    - \x1b[35mcheckra1n\x1b[39m\n");
    printf("\t--a11  \x1b[36mt8015\x1b[39m    - \x1b[35mcheckra1n\x1b[39m\n");
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
    
    printf("kIOReturnSuccess: %x\n", kIOReturnSuccess);
    printf("kIOUSBPipeStalled: %x\n", kIOUSBPipeStalled);
    printf("kUSBHostReturnPipeStalled: %x\n", kUSBHostReturnPipeStalled);
    printf("kIOReturnTimeout: %x\n", kIOReturnTimeout);
    printf("kIOUSBTransactionTimeout: %x\n", kIOUSBTransactionTimeout);
    printf("kIOReturnNotResponding: %x\n", kIOReturnNotResponding);
    
    if(!strcmp(argv[1], "--a10")) {
        devmode = 0x8010;
    } else if(!strcmp(argv[1], "--a11")) {
        devmode = 0x8015;
    } else {
        usage(argv);
        return -1;
    }
    
    log_dev();
    
    LOG_WAIT("Waiting for device in DFU mode...");
    
    while(get_device(DEVICE_DFU) != 0) {
        sleep(1);
    }
    LOG_DONE("CONNECTED");
    
#ifndef LOW_IPHONEOS_VERSION
    printf("%x, %s\n", client->devinfo.cpid, client->devinfo.srtg);
    
    if((client->devinfo.cpid == 0x8010)&&(devmode == 0x8010)){
        checkra1n_t8010_t8015(client, client->devinfo.cpid, payload); // checkra1n (for 14.x)
    } else if((client->devinfo.cpid == 0x8015)&&(devmode == 0x8015)){
        checkra1n_t8010_t8015(client, client->devinfo.cpid, payload); // checkra1n (for ~13.7)
    }
    
#else
    printf("%x\n", devmode);
    if(devmode == 0x8010){
        checkra1n_t8010_t8015(client, devmode, payload); // checkra1n (for 14.x)
    }
    
    if(devmode == 0x8015){
        checkra1n_t8010_t8015(client, devmode, payload); // checkra1n (for ~13.7)
    }
#endif
    
    
    return 0;
}


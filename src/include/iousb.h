#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>

#ifndef kUSBHostReturnPipeStalled
#define kUSBHostReturnPipeStalled (IOReturn)0xe0005000
#endif

#define DEVICE_DFU              (0x1227)
#define DEVICE_STAGE2           (0x1338)
#define DEVICE_PONGO            (0x4141)
#define DEVICE_RECOVERY_MODE_1  (0x1280)
#define DEVICE_RECOVERY_MODE_2  (0x1281)
#define DEVICE_RECOVERY_MODE_3  (0x1282)
#define DEVICE_RECOVERY_MODE_4  (0x1283)

/* LOG macro */
#define LOG_ERROR(x, ...) do { printf("\x1b[31m"x"\x1b[39m\n", ##__VA_ARGS__); } while(0)
#define LOG_EXPLOIT_NAME(x, ...) do { printf("\x1b[1m** \x1b[31mexploiting with "x"\x1b[39;0m\n", ##__VA_ARGS__); } while(0)
#define LOG_DONE(x, ...) do { printf("\x1b[31;1m"x"\x1b[39;0m\n", ##__VA_ARGS__); } while(0)
#define LOG_WAIT(x, ...) do { printf("\x1b[36m"x"\x1b[39m\n", ##__VA_ARGS__); } while(0)
#define LOG_PROGRESS(x, ...) do { printf("\x1b[32m"x"\x1b[39m\n", ##__VA_ARGS__); } while(0)

struct io_devinfo {
    unsigned int sdom;
    unsigned int cpid;
    unsigned int bdid;
    bool hasSRNM;
    bool hasPWND;
    char* srtg;
};

typedef struct io_client_p io_client_p;
typedef io_client_p* io_client_t;

struct io_client_p {
    IOUSBDeviceInterface320 **handle;
    CFRunLoopSourceRef async_event_source;
    unsigned int mode;
    unsigned int vid;
    int usb_interface;
    int usb_alt_interface;
    struct io_devinfo devinfo;
    bool hasSerialStr;
};

typedef struct {
    void *over1;
    unsigned int over1_len;
    void *over2;
    unsigned int over2_len;
    void *stage2;
    unsigned int stage2_len;
    void *pongoOS;
    unsigned int pongoOS_len;
} checkra1n_payload_t;

typedef struct {
    UInt32 wLenDone;
    IOReturn ret;
} transfer_t;

typedef transfer_t async_transfer_t;

int get_device(unsigned int mode);
void io_close(io_client_t client);
int io_open(io_client_t *pclient, uint16_t pid);
int io_reset(io_client_t client);
int get_device_time_stage(io_client_t *pclient, unsigned int time, uint16_t stage);

IOReturn io_abort_pipe_zero(io_client_t client);

transfer_t usb_ctrl_transfer(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length);
transfer_t usb_ctrl_transfer_with_time(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int time);
transfer_t async_usb_ctrl_transfer(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, async_transfer_t* transfer);
UInt32 async_usb_ctrl_transfer_with_cancel(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int ns_time);
UInt32 async_usb_ctrl_transfer_no_error(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length);
UInt32 async_usb_ctrl_transfer_with_cancel_noloop(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int ns_time);


void SNR(io_client_t client);

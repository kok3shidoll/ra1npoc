#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>

#include <iousb.h>


extern io_client_t client;

static const char *darwin_device_class = kIOUSBDeviceClassName;

static int nsleep(long nanoseconds) {
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = nanoseconds;
    return nanosleep(&req, &rem);
}

static int check_context(io_client_t client) {
    if (client == NULL || client->handle == NULL) {
        return -1;
    }
    return 0;
}

static void cfdictionary_set_short(CFMutableDictionaryRef dict, const void *key, SInt16 value)
{
    CFNumberRef numberRef;
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &value);
    if (numberRef) {
        CFDictionarySetValue(dict, key, numberRef);
        CFRelease(numberRef);
    }
}

static io_iterator_t io_get_iterator_for_pid(uint16_t pid) {
    
    IOReturn result;
    io_iterator_t iterator;
    CFMutableDictionaryRef matchingDict;
    
#ifdef IPHONEOS_ARM
    // Allows iOS to connect to iOS devices.
    darwin_device_class = "IOUSBHostDevice";
#endif
    
    matchingDict = IOServiceMatching(darwin_device_class);
    cfdictionary_set_short(matchingDict, CFSTR(kUSBVendorID), kAppleVendorID);
    cfdictionary_set_short(matchingDict, CFSTR(kUSBProductID), pid);
    
    result = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iterator);
    if (result != kIOReturnSuccess){
        return IO_OBJECT_NULL;
    }
    
    return iterator;
}

static void load_devinfo(io_client_t client, const char* str)
{
    if (!client || !str) {
        return;
    }
    
    char* ptr;
    char tmp[256];
    
    memset(&client->devinfo, '\0', sizeof(struct io_devinfo));
    memset(&tmp, '\0', 256);
    
    ptr = strstr(str, "SDOM:");
    if (ptr != NULL) {
        sscanf(ptr, "SDOM:%x", &client->devinfo.sdom);
    }
    
    ptr = strstr(str, "CPID:");
    if (ptr != NULL) {
        sscanf(ptr, "CPID:%x", &client->devinfo.cpid);
    }
    
    ptr = strstr(str, "BDID:");
    if (ptr != NULL) {
        sscanf(ptr, "BDID:%x", &client->devinfo.bdid);
    }
    
    ptr = strstr(str, "SRNM:[");
    if(ptr != NULL) {
        client->devinfo.hasSRNM = TRUE;
    } else {
        client->devinfo.hasSRNM = FALSE;
    }
    
    ptr = strstr(str, "PWND:[");
    if(ptr != NULL) {
        client->devinfo.hasPWND = TRUE;
    } else {
        client->devinfo.hasPWND = FALSE;
    }
    
    tmp[0] = '\0';
    ptr = strstr(str, "SRTG:[");
    if(ptr != NULL) {
        sscanf(ptr, "SRTG:[%s]", tmp);
        ptr = strrchr(tmp, ']');
        if(ptr != NULL) {
            *ptr = '\0';
        }
        client->devinfo.srtg = strdup(tmp);
    }
}

void io_close(io_client_t client){
    if (client->handle) {
        (*client->handle)->USBDeviceClose(client->handle);
        (*client->handle)->Release(client->handle);
        client->handle = NULL;
    }
    if(client->async_event_source) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), client->async_event_source, kCFRunLoopDefaultMode);
        CFRelease(client->async_event_source);
    }
    free(client);
    client = NULL;
}

// for iOS 10 or lower
void SNR(io_client_t client){
    transfer_t result;
    uint8_t size;
    
    unsigned char buf[0x100];
    unsigned char str[0x100];
    bzero(str, 0x100);
    bzero(buf, 0x100);
    
    if(client->devinfo.srtg == NULL){
        result = usb_ctrl_transfer(client, 0x80, 6, 0x0306, 0x040a, buf, 0x100); // 8950 or up (PWND)
        if(result.ret != kIOReturnSuccess) return;
        size = *(uint8_t*)buf;
        for(int i=0;i<(size/2);i++){
            str[i] = *(uint8_t*)(buf+2+(i*2));
        }
        load_devinfo(client, (const char*)str);
    }
    
    if(client->devinfo.srtg == NULL){
        result = usb_ctrl_transfer(client, 0x80, 6, 0x0304, 0x040a, buf, 0x100); // 8950 or up
        if(result.ret != kIOReturnSuccess) return;
        size = *(uint8_t*)buf;
        for(int i=0;i<(size/2);i++){
            str[i] = *(uint8_t*)(buf+2+(i*2));
        }
        load_devinfo(client, (const char*)str);
    }
    
    if(client->devinfo.srtg == NULL){
        bzero(buf, 0x100);
        bzero(str, 0x80);
        result = usb_ctrl_transfer(client, 0x80, 6, 0x0303, 0x040a, buf, 0x100); // 8930
        if(result.ret != kIOReturnSuccess) return;
        size = *(uint8_t*)buf;
        for(int i=0;i<(size/2);i++){
            str[i] = *(uint8_t*)(buf+2+(i*2));
        }
        load_devinfo(client, (const char*)str);
    }
    
    if(client->devinfo.srtg != NULL){
        client->hasSerialStr = TRUE;
    }
}

static void io_async_cb(void *refcon, IOReturn ret, void *arg_0)
{
    async_transfer_t* transfer = refcon;
    
    if(transfer != NULL) {
        transfer->ret = ret;
        memcpy(&transfer->wLenDone, &arg_0, sizeof(transfer->wLenDone));
        CFRunLoopStop(CFRunLoopGetCurrent());
    }
}

int io_open(io_client_t *pclient, uint16_t pid){
    io_service_t service = IO_OBJECT_NULL;
    io_iterator_t iterator;
    
    IOReturn result;
    io_client_t _client;
    IOCFPlugInInterface **plug = NULL;
    SInt32 score;
    UInt16 mode;
    UInt32 locationID;
    CFStringRef serialString;
    
    client = NULL;
    iterator = io_get_iterator_for_pid(pid);
    
    if (iterator != IO_OBJECT_NULL) {
        while((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
            break;
        }
        IOObjectRelease(iterator);
        
        _client = (io_client_t) calloc(1, sizeof(struct io_client_p));
        
        result = IOCreatePlugInInterfaceForService(service, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score);
        if (result != kIOReturnSuccess) {
            IOObjectRelease(service);
            free(_client);
            return -1;
        }
        
        // Older iOS versions (such as iOS 10) can't get the serial number, so don't get it.
        char serial_str[256];
        serial_str[0] = '\0';
        serialString = IORegistryEntryCreateCFProperty(service, CFSTR(kUSBSerialNumberString), kCFAllocatorDefault, 0);
        if (serialString) {
            CFStringGetCString(serialString, serial_str, sizeof(serial_str), kCFStringEncodingUTF8);
            CFRelease(serialString);
            load_devinfo(_client, serial_str);
            _client->hasSerialStr = TRUE;
        } else {
            _client->hasSerialStr = FALSE;
        }
        IOObjectRelease(service);
        
        // Create the device interface
        result = (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID320), (LPVOID *)&(_client->handle));
        IODestroyPlugInInterface(plug);
        if (result != kIOReturnSuccess) {
            free(_client);
            return -1;
        }
        
        (*_client->handle)->GetDeviceProduct(_client->handle, &mode);
        (*_client->handle)->GetLocationID(_client->handle, &locationID);
        _client->mode = mode;
        
        result = (*_client->handle)->USBDeviceOpen(_client->handle);
        if (result != kIOReturnSuccess) {
            (*_client->handle)->Release(_client->handle);
            free(_client);
            return -1;
        }
        
        result = (*_client->handle)->SetConfiguration(_client->handle, 1);
        if (result != kIOReturnSuccess) {
            free(_client);
            return -1;
        }
        
        result = (*_client->handle)->CreateDeviceAsyncEventSource(_client->handle, &_client->async_event_source);
        if (result != kIOReturnSuccess) {
            free(_client);
            return -1;
        }
        
        CFRunLoopAddSource(CFRunLoopGetCurrent(), _client->async_event_source, kCFRunLoopDefaultMode);
        
        *pclient = _client;
        
        return 0;
    }
    
    return -1;
}

int get_device(unsigned int mode) {
    
    if(client) {
        io_close(client);
        client = NULL;
    }
    
    io_open(&client, mode);
    if(!client) {
        return -1;
    }
    
    if(client->mode != mode){
        io_close(client);
        client = NULL;
        return -1;
    }
    
    return 0;
}

int io_reset(io_client_t client) {
    
    if (check_context(client) != 0)
        return -1;
    
    IOReturn result;
    
    result = (*client->handle)->ResetDevice(client->handle);
    if (result != kIOReturnSuccess && result != kIOReturnNotResponding) {
        return -1;
    }
    
    result = (*client->handle)->USBDeviceReEnumerate(client->handle, 0);
    if (result != kIOReturnSuccess && result != kIOReturnNotResponding) {
        // error re-enumerating device: result (ignored)
    }
    
    return 0;
}

int get_device_time_stage(io_client_t *pclient, unsigned int time, uint16_t stage){
    
    for(int i=0; i<time; i++){
        if(*pclient) {
            io_close(*pclient);
            *pclient = NULL;
        }
        if (io_open(pclient, stage) != 0) {
            // Connection failed. Waiting 1 sec before retry.
            sleep(1);
        } else {
            return 0;
        }
    }
    return -1;
}

transfer_t usb_ctrl_transfer(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length)
{
    transfer_t result;
    IOUSBDevRequest req;
    
    bzero(&result, sizeof(transfer_t));
    bzero(&req, sizeof(req));
    
    req.bmRequestType     = bm_request_type;
    req.bRequest          = b_request;
    req.wValue            = OSSwapLittleToHostInt16(w_value);
    req.wIndex            = OSSwapLittleToHostInt16(w_index);
    req.wLength           = OSSwapLittleToHostInt16(w_length);
    req.pData             = data;
    
    result.ret = (*client->handle)->DeviceRequest(client->handle, &req);
    result.wLenDone = req.wLenDone;
    
    return result;
}

transfer_t usb_ctrl_transfer_with_time(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int time)
{
    transfer_t result;
    IOUSBDevRequestTO req;
    
    bzero(&result, sizeof(transfer_t));
    bzero(&req, sizeof(req));
    
    req.bmRequestType     = bm_request_type;
    req.bRequest          = b_request;
    req.wValue            = OSSwapLittleToHostInt16(w_value);
    req.wIndex            = OSSwapLittleToHostInt16(w_index);
    req.wLength           = OSSwapLittleToHostInt16(w_length);
    req.pData             = data;
    req.noDataTimeout     = time;
    req.completionTimeout = time;

    result.ret = (*client->handle)->DeviceRequestTO(client->handle, &req);
    result.wLenDone = req.wLenDone;
    
    return result;
}

IOReturn io_abort_pipe_zero(io_client_t client){
    return (*client->handle)->USBDeviceAbortPipeZero(client->handle);
}

transfer_t async_usb_ctrl_transfer(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, async_transfer_t* transfer)
{
    transfer_t result;
    IOUSBDevRequest req;
    
    bzero(&result, sizeof(transfer_t));
    bzero(&req, sizeof(req));
    
    req.bmRequestType     = bm_request_type;
    req.bRequest          = b_request;
    req.wValue            = OSSwapLittleToHostInt16(w_value);
    req.wIndex            = OSSwapLittleToHostInt16(w_index);
    req.wLength           = OSSwapLittleToHostInt16(w_length);
    req.pData             = data;
    
    result.ret = (*client->handle)->DeviceRequestAsync(client->handle, &req, io_async_cb, transfer);
    result.wLenDone = req.wLenDone;
    
    return result;
}

UInt32 async_usb_ctrl_transfer_with_cancel(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int ns_time)
{
    transfer_t result;
    IOReturn error;
    async_transfer_t transfer;
    
    bzero(&transfer, sizeof(async_transfer_t));
    
    result = async_usb_ctrl_transfer(client, bm_request_type, b_request, w_value, w_index, data, w_length, &transfer);
    if(result.ret != kIOReturnSuccess) {
        return result.ret;
    }
    nsleep(ns_time);
    
    error = io_abort_pipe_zero(client);
    if(error != kIOReturnSuccess) {
        return -1;
    }
    
    while(transfer.ret != kIOReturnAborted){
        CFRunLoopRun();
    }
    
    return transfer.wLenDone;
}

UInt32 async_usb_ctrl_transfer_no_error(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length)
{
    async_transfer_t transfer;
    bzero(&transfer, sizeof(async_transfer_t));

    async_usb_ctrl_transfer(client, bm_request_type, b_request, w_value, w_index, data, w_length, &transfer);
    return transfer.wLenDone;
}

UInt32 async_usb_ctrl_transfer_with_cancel_noloop(io_client_t client, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int ns_time)
{
    async_transfer_t transfer;
    bzero(&transfer, sizeof(async_transfer_t));

    async_usb_ctrl_transfer(client, bm_request_type, b_request, w_value, w_index, data, w_length, &transfer);
    nsleep(ns_time);
    io_abort_pipe_zero(client);

    return transfer.wLenDone;
}

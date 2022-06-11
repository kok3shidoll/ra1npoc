/*
 * ra1npoc - main.c
 *
 * Copyright (c) 2021 - 2022 dora2ios
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <io/iousb.h>

#include <common/common.h>
#include <common/pongo_config.h>
#include <common/list.h>

#include <exploit/checkm8_arm64.h>

#ifdef BUILTIN_PAYLOAD
#include <getopt.h>

#include "payload/pongoOS.h"

#if defined(S5L8960_PAYLOAD)
#include "payload/s5l8960.h"
#endif /* S5L8960_PAYLOAD */

#if defined(T7000_PAYLOAD)
#include "payload/t7000.h"
#endif /* T7000_PAYLOAD */

#if defined(T7001_PAYLOAD)
#include "payload/t7001.h"
#endif /* T7001_PAYLOAD */

#if (defined(S8000_PAYLOAD) || defined(S8003_PAYLOAD))
#include "payload/s8000.h"
#endif /* S8000_PAYLOAD */

#if defined(S8001_PAYLOAD)
#include "payload/s8001.h"
unsigned char special_pongoOS[MAX_HAXX_SIZE];
#endif /* S8001_PAYLOAD */

#if defined(T8010_PAYLOAD)
#include "payload/t8010.h"
#endif /* T8010_PAYLOAD */

#if defined(T8011_PAYLOAD)
#include "payload/t8011.h"
#endif /* T8011_PAYLOAD */

#if defined(T8012_PAYLOAD)
#include "payload/t8012.h"
#endif /* T8012_PAYLOAD */

#if defined(T8015_PAYLOAD)
#include "payload/t8015.h"
#endif /* T8015_PAYLOAD */

#endif /* BUILTIN_PAYLOAD */

#if defined(KPF_FLAGS_PTR) && defined(BOOTARGS_STR_PTR)
int8_t kpf_flags = checkrain_option_none;
const char* bootargs = NULL;
#endif

io_client_t client;
checkra1n_payload_t payload;
bool special_pongo = false;

extern bool debug_enabled;

#ifndef BUILTIN_PAYLOAD
static int open_file(char *file, unsigned int *sz, unsigned char **buf)
{
    FILE *fd = fopen(file, "r");
    if (!fd) {
        ERROR("opening %s", file);
        return -1;
    }
    
    fseek(fd, 0, SEEK_END);
    *sz = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    *buf = malloc(*sz);
    if (!*buf) {
        ERROR("allocating file buffer");
        fclose(fd);
        return -1;
    }
    
    fread(*buf, *sz, 1, fd);
    fclose(fd);
    
    return 0;
}
#endif /* BUILTIN_PAYLOAD */

#ifdef BUILTIN_PAYLOAD
static void list(void)
{
    printf("Devices list:\n");
#if defined(S5L8960_CODE) && defined(S5L8960_PAYLOAD)
    printf("\t\x1b[36ms5l8960x\x1b[39m - \x1b[35mApple A7\x1b[39m\n");
#endif /* S5L8960 */
#if defined(T7000_CODE) && defined(T7000_PAYLOAD)
    printf("\t\x1b[36mt7000   \x1b[39m - \x1b[35mApple A8\x1b[39m\n");
#endif /* T7000 */
#if defined(T7001_CODE) && defined(T7001_PAYLOAD)
    printf("\t\x1b[36mt7001   \x1b[39m - \x1b[35mApple A8X\x1b[39m\n");
#endif /* T7001 */
#if defined(S8000_CODE) && defined(S8000_PAYLOAD)
    printf("\t\x1b[36ms8000   \x1b[39m - \x1b[35mApple A9 (Samsung)\x1b[39m\n");
#endif /* S8000 */
#if defined(S8003_CODE) && defined(S8003_PAYLOAD)
    printf("\t\x1b[36ms8003   \x1b[39m - \x1b[35mApple A9 (TSMC)\x1b[39m\n");
#endif /* S8003 */
#if defined(S8001_CODE) && defined(S8001_PAYLOAD)
    printf("\t\x1b[36ms8001   \x1b[39m - \x1b[35mApple A9X\x1b[39m\n");
#endif /* S8001 */
#if defined(T8010_CODE) && defined(T8010_PAYLOAD)
    printf("\t\x1b[36mt8010   \x1b[39m - \x1b[35mApple A10 Fusion\x1b[39m\n");
#endif /* T8010 */
#if defined(T8011_CODE) && defined(T8011_PAYLOAD)
    printf("\t\x1b[36mt8011   \x1b[39m - \x1b[35mApple A10X Fusion\x1b[39m\n");
#endif /* T8011 */
#if defined(T8012_CODE) && defined(T8012_PAYLOAD)
    printf("\t\x1b[36mt8012   \x1b[39m - \x1b[35mApple T2\x1b[39m\n");
#endif /* T8012 */
#if defined(T8015_CODE) && defined(T8015_PAYLOAD)
    printf("\t\x1b[36mt8015   \x1b[39m - \x1b[35mApple A11 Bionic\x1b[39m\n");
#endif /* T8015 */
}
#endif

static void usage(char** argv)
{
#ifndef BUILTIN_PAYLOAD
    printf("Usage: %s [option] overwrite1 overwrite2 stage2 payload.bin\n", argv[0]);
#else
    printf("Usage: %s [option]\n", argv[0]);
#endif /* BUILTIN_PAYLOAD */
    
#ifndef BUILTIN_PAYLOAD
    
#if defined(S5L8960_CODE)
    printf("\t--a7   \x1b[36ms5l8960x\x1b[39m - \x1b[35mApple A7\x1b[39m\n");
#endif /* S5L8960 */
#if defined(T7000_CODE)
    printf("\t--a8   \x1b[36mt7000   \x1b[39m - \x1b[35mApple A8\x1b[39m\n");
#endif /* T7000 */
#if defined(T7001_CODE)
    printf("\t--a8x  \x1b[36mt7001   \x1b[39m - \x1b[35mApple A8X\x1b[39m\n");
#endif /* T7001 */
#if defined(S8000_CODE)
    printf("\t--a9   \x1b[36ms8000   \x1b[39m - \x1b[35mApple A9 (Samsung)\x1b[39m\n");
#endif /* S8000 */
#if defined(S8003_CODE)
    printf("\t--a9m  \x1b[36ms8003   \x1b[39m - \x1b[35mApple A9 (TSMC)\x1b[39m\n");
#endif /* S8003 */
#if defined(S8001_CODE)
    printf("\t--a9x  \x1b[36ms8001   \x1b[39m - \x1b[35mApple A9X\x1b[39m\n");
#endif /* S8001 */
#if defined(T8010_CODE)
    printf("\t--a10  \x1b[36mt8010   \x1b[39m - \x1b[35mApple A10 Fusion\x1b[39m\n");
#endif /* T8010 */
#if defined(T8011_CODE)
    printf("\t--a10x \x1b[36mt8011   \x1b[39m - \x1b[35mApple A10X Fusion\x1b[39m\n");
#endif /* T8011 */
#if defined(T8012_CODE)
    printf("\t--t2   \x1b[36mt8011   \x1b[39m - \x1b[35mApple T2\x1b[39m\n");
#endif /* T8012 */
#if defined(T8015_CODE)
    printf("\t--a11  \x1b[36mt8015   \x1b[39m - \x1b[35mApple A11 Bionic\x1b[39m\n");
#endif /* T8015 */
    
#else
    printf("  -h, --help\t\t\t\x1b[36mshow usage\x1b[39m\n");
    printf("  -l, --list\t\t\t\x1b[36mshow list of supported devices\x1b[39m\n");
    printf("  -v, --verbose\t\t\t\x1b[36menable verbose boot\x1b[39m\n");
    printf("  -c, --cleandfu\t\t\x1b[36muse cleandfu [BETA]\x1b[39m\n");
    printf("  -d, --debug\t\t\t\x1b[36menable debug log\x1b[39m\n");
    printf("  -e, --extra-bootargs <args>\t\x1b[36mset extra bootargs\x1b[39m\n");
    printf("  -s, --special\t\t\t\x1b[36muse special pongo_2.5.0-0cb6126f\x1b[39m\n");
#endif /* BUILTIN_PAYLOAD */
    
    printf("\n");
}

int main(int argc, char** argv)
{
    int ret = 0;
    
    memset(&payload, '\0', sizeof(checkra1n_payload_t));
    LOG_NOFUNC("* checkRAIN clone v2.1.4 for iOS by interception");
#ifdef BUILTIN_PAYLOAD
    LOG_NOFUNC("[BUILTIN] v0.12.4");
    //LOG_NOFUNC("[COMMIT] %s", "");
#endif
    
#ifndef BUILTIN_PAYLOAD
    uint16_t devmode=0;
#else
    bool useRecovery = false;
    bool verboseBoot = false;
    char* extraBootArgs = NULL;
    
#endif /* !BUILTIN_PAYLOAD */
 
#ifndef BUILTIN_PAYLOAD
    if(argc != 6) {
        usage(argv);
        return -1;
    }
    
    if(!strcmp(argv[1], "--a11")) {
        devmode = 0x8015;
    }
    if(!strcmp(argv[1], "--t2")) {
        devmode = 0x8012;
    }
    if(!strcmp(argv[1], "--a10x")) {
        devmode = 0x8011;
    }
    if(!strcmp(argv[1], "--a10")) {
        devmode = 0x8010;
    }
    if(!strcmp(argv[1], "--a9x")) {
        devmode = 0x8001;
    }
    if(!strcmp(argv[1], "--a9m")) {
        devmode = 0x8003;
    }
    if(!strcmp(argv[1], "--a9")) {
        devmode = 0x8000;
    }
    if(!strcmp(argv[1], "--a8x")) {
        devmode = 0x7001;
    }
    if(!strcmp(argv[1], "--a8")) {
        devmode = 0x7000;
    }
    if(!strcmp(argv[1], "--a7")) {
        devmode = 0x8960;
    }
    if(!devmode) {
        usage(argv);
        return -1;
    }
    
#ifdef DEBUG
    debug_enabled = true;
#endif /* DEBUG */
    
#else /* !BUILTIN_PAYLOAD */
    int opt = 0;
    static struct option longopts[] = {
        { "help",           no_argument,       NULL, 'h' },
        { "list",           no_argument,       NULL, 'l' },
        { "verbose",        no_argument,       NULL, 'v' },
        { "cleandfu",       no_argument,       NULL, 'c' },
        { "debug",          no_argument,       NULL, 'd' },
        { "extra-bootargs", required_argument, NULL, 'e' },
        { "special",        no_argument,       NULL, 's' },
        { NULL, 0, NULL, 0 }
    };
    
    while ((opt = getopt_long(argc, argv, "hlvdce:s", longopts, NULL)) > 0) {
        switch (opt) {
            case 'h':
                usage(argv);
                return 0;
                
            case 'l':
                list();
                return 0;
                
            case 'v':
                verboseBoot = true;
                break;
                
            case 'd':
                debug_enabled = true;
                DEBUGLOG("enabled: debug log");
                break;
                
            case 'c':
                useRecovery = true;
                break;
                
            case 'e':
                if (optarg) {
                    extraBootArgs = strdup(optarg);
                    LOG("extraBootArgs: [%s]", extraBootArgs);
                }
                break;
                
            case 's':
                special_pongo = true;
                break;
                
            default:
                usage(argv);
                return -1;
        }
    }
    
    if(useRecovery) {
        if(enter_dfu_via_recovery(client)) {
            return -1;
        }
    }
    
#endif /* BUILTIN_PAYLOAD */
    
    LOG("Waiting for device in DFU mode...");
    while(get_device(DEVICE_DFU, true)) {
        sleep(1);
    }
    
    LOG("CONNECTED: DFU mode");
    sleep(2);
    
    if(client->hasSerialStr == false){
        read_serial_number(client); // For iOS 10 and lower
    }
    
    DEBUGLOG("CPID: 0x%02x, STRG: [%s]", client->devinfo.cpid, client->devinfo.srtg);
    
    if(client->hasSerialStr != true){
        ERROR("serial number was not found!");
        return -1;
    }
    
#ifndef BUILTIN_PAYLOAD
    if(client->devinfo.cpid != devmode) {
        ERROR("option does not match the connected device!");
        return -1;
    }
    
    // load payload
    char *over1_path    = argv[2];
    char *over2_path    = argv[3];
    char *stage2_path   = argv[4];
    char *pongoOS_path  = argv[5];
    
    if(open_file(over1_path,   &payload.over1_len,   &payload.over1)   != 0) return -1;
    if(open_file(over2_path,   &payload.over2_len,   &payload.over2)   != 0) return -1;
    if(open_file(stage2_path,  &payload.stage2_len,  &payload.stage2)  != 0) return -1;
    if(open_file(pongoOS_path, &payload.pongoOS_len, &payload.pongoOS) != 0) return -1;
#endif /* !BUILTIN_PAYLOAD */
    
    switch(client->devinfo.cpid) {
            
#if defined(S5L8960_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S5L8960_PAYLOAD))
        case 0x8960:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = s5l8960_overwrite1_len;
            payload.over2_len = s5l8960_overwrite2_len;
            payload.stage2_len = s5l8960_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = s5l8960_overwrite1;
            payload.over2 = s5l8960_overwrite2;
            payload.stage2 = s5l8960_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* S5L8960 */
            
#if defined(T7000_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T7000_PAYLOAD))
        case 0x7000:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = 0;
            payload.over2_len = t7000_overwrite2_len;
            payload.stage2_len = t7000_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = NULL;
            payload.over2 = t7000_overwrite2;
            payload.stage2 = t7000_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* T7000 */
 
#if defined(T7001_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T7001_PAYLOAD))
        case 0x7001:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = 0;
            payload.over2_len = t7001_overwrite2_len;
            payload.stage2_len = t7001_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = NULL;
            payload.over2 = t7001_overwrite2;
            payload.stage2 = t7001_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* T7001 */
            
#if defined(S8000_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8000_PAYLOAD))
        case 0x8000:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = 0;
            payload.over2_len = s8000_overwrite2_len;
            payload.stage2_len = s8000_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = NULL;
            payload.over2 = s8000_overwrite2;
            payload.stage2 = s8000_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* S8000 */
            
#if defined(S8003_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8003_PAYLOAD))
        case 0x8003:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = 0;
            payload.over2_len = s8000_overwrite2_len;
            payload.stage2_len = s8000_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = NULL;
            payload.over2 = s8000_overwrite2;
            payload.stage2 = s8000_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* S8003 */
            
#if defined(S8001_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8001_PAYLOAD))
        case 0x8001:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = s8001_overwrite1_len;
            payload.over2_len = s8001_overwrite2_len;
            payload.stage2_len = s8001_stage2_len;
            payload.over1 = s8001_overwrite1;
            payload.over2 = s8001_overwrite2;
            payload.stage2 = s8001_stage2;
            if(special_pongo == true) {
                size_t pongo_size = 0;
                
                memset(&special_pongoOS, '\0', MAX_HAXX_SIZE);
                
                // 1, copy pongo
                if(pongo_2_5_0_0cb6126f_bin_len > MAX_HAXX_SIZE) {
                    ERROR("overflow: %s", "pongo.bin");
                    return -1;
                }
                memcpy(special_pongoOS, pongo_2_5_0_0cb6126f_bin, pongo_2_5_0_0cb6126f_bin_len);
                pongo_size += pongo_2_5_0_0cb6126f_bin_len;
                
                // 2, add auto-boot mark
                unsigned char auto_boot_mark[] =
                {
                    0x61, 0x75, 0x74, 0x6f,
                    0x62, 0x6f, 0x6f, 0x74,
                    0x00, 0x00, 0x20, 0x00,
                    0x00, 0x00, 0x00, 0x00
                };
                size_t auto_boot_mark_size = 0x16;
                
                if(pongo_size + auto_boot_mark_size > MAX_HAXX_SIZE) {
                    ERROR("overflow: %s", "auto_boot_mark");
                    return -1;
                }
                memcpy(special_pongoOS+pongo_size, auto_boot_mark, 16);
                pongo_size += 16;
                
                // 3, copy kpf
                if(pongo_size + KPF_SIZE > MAX_HAXX_SIZE) {
                    ERROR("overflow: %s", "kpf");
                    return -1;
                }
                memcpy(special_pongoOS+pongo_size, pongoOS+KPF_LOCATION, KPF_SIZE);
                pongo_size += KPF_SIZE;
                
                // 4, add RDSK mark
                unsigned char RDSK_mark[] =
                {
                    0x52, 0x44, 0x53, 0x4B,
                    0x52, 0x44, 0x53, 0x4B,
                    0x00, 0x00, 0x10, 0x00
                };
                size_t RDSK_mark_size = 0xc;
                
                pongo_size = RDSK_LOCATION - SPECIAL_HAXX - RDSK_mark_size;
                if(pongo_size + RDSK_mark_size > MAX_HAXX_SIZE) {
                    ERROR("overflow: %s", "rdsk_mark");
                    return -1;
                }
                memcpy(special_pongoOS+pongo_size, RDSK_mark, RDSK_mark_size);
                pongo_size += RDSK_mark_size;
                
                // 5, copy rdsk
                if(pongo_size + RDSK_SIZE > MAX_HAXX_SIZE) {
                    ERROR("overflow: %s", "rdsk");
                    return -1;
                }
                memcpy(special_pongoOS+pongo_size, pongoOS+RDSK_LOCATION, RDSK_SIZE);
                pongo_size += RDSK_SIZE;
                
                // 6, set flags etc...?
                if(pongo_size + BLANK_SIZE > MAX_HAXX_SIZE) {
                    ERROR("overflow: %s", "BLANK_SIZE");
                    return -1;
                }
                memcpy(special_pongoOS+pongo_size, pongoOS+RDSK_LOCATION+RDSK_SIZE, BLANK_SIZE);
                pongo_size += BLANK_SIZE;
                
                DEBUGLOG("use: special pongoOS");
                payload.pongoOS = special_pongoOS;
                payload.pongoOS_len = pongo_size;
                
            } else {
                payload.pongoOS = pongoOS;
                payload.pongoOS_len = 0x3F2F8; // pongoOS_len
                client->devinfo.checkm8_flag |= NO_AUTOBOOT;
            }
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* S8001 */
            
#if defined(T8010_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8010_PAYLOAD))
        case 0x8010:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = t8010_overwrite1_len;
            payload.over2_len = t8010_overwrite2_len;
            payload.stage2_len = t8010_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = t8010_overwrite1;
            payload.over2 = t8010_overwrite2;
            payload.stage2 = t8010_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* T8010 */
            
#if defined(T8011_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8011_PAYLOAD))
        case 0x8011:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = t8011_overwrite1_len;
            payload.over2_len = t8011_overwrite2_len;
            payload.stage2_len = t8011_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = t8011_overwrite1;
            payload.over2 = t8011_overwrite2;
            payload.stage2 = t8011_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* T8011 */
            
#if defined(T8012_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8012_PAYLOAD))
        case 0x8012:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = t8012_overwrite1_len;
            payload.over2_len = t8012_overwrite2_len;
            payload.stage2_len = t8012_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = t8012_overwrite1;
            payload.over2 = t8012_overwrite2;
            payload.stage2 = t8012_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* T8012 */
            
#if defined(T8015_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8015_PAYLOAD))
        case 0x8015:
#ifdef BUILTIN_PAYLOAD
            payload.over1_len = t8015_overwrite1_len;
            payload.over2_len = t8015_overwrite2_len;
            payload.stage2_len = t8015_stage2_len;
            payload.pongoOS_len = pongoOS_len;
            payload.over1 = t8015_overwrite1;
            payload.over2 = t8015_overwrite2;
            payload.stage2 = t8015_stage2;
            payload.pongoOS = pongoOS;
#endif /* BUILTIN_PAYLOAD */
            break;
#endif /* T8015 */
            
        default:
            ERROR("This device is not supported!");
            return -1;
    }
    
#if defined(BUILTIN_PAYLOAD) && (defined(KPF_FLAGS_PTR) && defined(BOOTARGS_STR_PTR))
    char str[MAX_BOOTARGS_LEN];
    memset(&str, 0x0, MAX_BOOTARGS_LEN);
    
    if(strlen(DEFAULT_BOOTARGS) > MAX_BOOTARGS_LEN) {
        ERROR("DEFAULT_BOOTARGS is too large!");
        return -1;
    }
    sprintf(str, "%s", DEFAULT_BOOTARGS);
    
    if(verboseBoot == true) {
        if((strlen(str) + strlen(" -v")) > MAX_BOOTARGS_LEN) {
            ERROR("bootArgs is too large!");
            return -1;
        }
        sprintf(str, "%s -v", str);
        
        checkrain_set_option(kpf_flags, checkrain_option_verbose_boot, 1);
        DEBUGLOG("kpf_flags: %x", kpf_flags);
        LOG("enable: verbose boot");
    }
    if(extraBootArgs != NULL) {
        if((strlen(str) + strlen(extraBootArgs)) > MAX_BOOTARGS_LEN) {
            ERROR("bootArgs is too large!");
            return -1;
        }
        sprintf(str, "%s %s", str, extraBootArgs);
    }
    bootargs = str;
    LOG("bootArgs: %s", bootargs);
#endif
    
    int flags = client->devinfo.checkm8_flag; // because checkm8_flag(s) will be lost
    
    ret = checkm8_arm64(client, payload, flags);
    
    if((ret == 0) && (flags & NO_AUTOBOOT))
        LOG("note: probably pongoOS booted, but there is still work to be done.\nYou have to sending rdsk and kpf via pongoterm.");
    
    return ret;
}


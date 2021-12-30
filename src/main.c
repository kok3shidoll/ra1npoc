/*
 * ra1npoc - main.c
 *
 * Copyright (c) 2021 dora2ios
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
#include <common/list.h>

#include <soc/t8010_t8015.h>
#include <soc/t7000_s8000.h>
#include <soc/s5l8960x.h>

#ifdef BUILTIN_PAYLOAD

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

#if defined(S8000_PAYLOAD)
#include "payload/s8000.h"
#endif /* S8000_PAYLOAD */

#if defined(S8003_PAYLOAD)
#include "payload/s8003.h"
#endif /* S8003_PAYLOAD */

#if defined(S8001_PAYLOAD)
#include "payload/s8001.h"
#endif /* S8001_PAYLOAD */

#if defined(T8010_PAYLOAD)
#include "payload/t8010.h"
#endif /* T8010_PAYLOAD */

#if defined(T8011_PAYLOAD)
#include "payload/t8011.h"
#endif /* T8011_PAYLOAD */

#if defined(T8015_PAYLOAD)
#include "payload/t8015.h"
#endif /* T8015_PAYLOAD */

#endif /* BUILTIN_PAYLOAD */

#if defined(KPF_FLAGS_PTR) && defined(BOOTARGS_STR_PTR)
int8_t kpf_flags = checkrain_option_none;
const char* bootargs = DEFAULT_BOOTARGS;
#endif

io_client_t client;
checkra1n_payload_t payload;

#ifndef BUILTIN_PAYLOAD
static int open_file(char *file, unsigned int *sz, void **buf)
{
    FILE *fd = fopen(file, "r");
    if (!fd) {
        ERROR("[%s] error opening %s", __FUNCTION__, file);
        return -1;
    }
    
    fseek(fd, 0, SEEK_END);
    *sz = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    *buf = malloc(*sz);
    if (!*buf) {
        ERROR("[%s] error allocating file buffer", __FUNCTION__);
        fclose(fd);
        return -1;
    }
    
    fread(*buf, *sz, 1, fd);
    fclose(fd);
    
    return 0;
}
#endif /* BUILTIN_PAYLOAD */

static void usage(char** argv)
{
#ifndef BUILTIN_PAYLOAD
    printf("Usage: %s [option] over1 over2 stage2 payload\n", argv[0]);
#else
    printf("Usage: %s [option]\n", argv[0]);
#endif /* BUILTIN_PAYLOAD */
    
#if defined(S5L8960_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S5L8960_PAYLOAD))
    printf("\t--a7   \x1b[36ms5l8960x\x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* S5L8960 */
    
#if defined(T7000_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T7000_PAYLOAD))
    printf("\t--a8   \x1b[36mt7000   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* T7000 */
    
#if defined(T7001_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T7001_PAYLOAD))
    printf("\t--a8x  \x1b[36mt7001   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* T7001 */
    
#if defined(S8000_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8000_PAYLOAD))
    printf("\t--a9   \x1b[36ms8000   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* S8000 */
    
#if defined(S8003_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8003_PAYLOAD))
    printf("\t--a9m  \x1b[36ms8003   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* S8003 */
    
#if defined(S8001_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8001_PAYLOAD))
    printf("\t--a9x  \x1b[36ms8001   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* S8001 */
    
#if defined(T8010_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8010_PAYLOAD))
    printf("\t--a10  \x1b[36mt8010   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* T8010 */
    
#if defined(T8011_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8011_PAYLOAD))
    printf("\t--a10x \x1b[36mt8011   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* T8011 */
    
#if defined(T8015_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8015_PAYLOAD))
    printf("\t--a11  \x1b[36mt8015   \x1b[39m - \x1b[35mcheckra1n\x1b[39m\n");
#endif /* T8015 */
    
    printf("\n");
}

int main(int argc, char** argv)
{
    int arg=0;
    uint16_t devmode=0;
    
#ifndef BUILTIN_PAYLOAD
    arg = 6;
#else
    arg = 2;
#endif /* BUILTIN_PAYLOAD */
    
    if(argc != arg) {
        usage(argv);
        return -1;
    }
    
    if(!strcmp(argv[1], "--a11")) {
        devmode = 0x8015;
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
    
    memset(&payload, '\0', sizeof(checkra1n_payload_t));
    LOG("* checkRAIN clone v2.0 for iOS by interception");
    
    LOG("[%s] Waiting for device in DFU mode...", __FUNCTION__);
    while(get_device(DEVICE_DFU, true) != 0) {
        sleep(1);
    }
    
    LOG("[%s] CONNECTED", __FUNCTION__);
    
    if(client->hasSerialStr == false){
        read_serial_number(client); // For iOS 10 and lower
    }
    
    DEBUGLOG("[%s] CPID: 0x%02x, STRG: [%s]", __FUNCTION__, client->devinfo.cpid, client->devinfo.srtg);
    
    if(client->hasSerialStr != true){
        ERROR("[%s] Serial number was not found!", __FUNCTION__);
        return -1;
    }
    
#ifndef BUILTIN_PAYLOAD
    char *over1_path = argv[2];
    char *over2_path = argv[3];
    char *stage2_path = argv[4];
    char *pongoOS_path = argv[5];
    
    if(open_file(over1_path, &payload.over1_len, &payload.over1) != 0) return -1;
    if(open_file(over2_path, &payload.over2_len, &payload.over2) != 0) return -1;
    if(open_file(stage2_path, &payload.stage2_len, &payload.stage2) != 0) return -1;
    if(open_file(pongoOS_path, &payload.pongoOS_len, &payload.pongoOS) != 0) return -1;
#else
    
#if defined(S5L8960_PAYLOAD)
    if((client->devinfo.cpid) == 0x8960) {
        payload.over1_len = s5l8960_overwrite1_len;
        payload.over2_len = s5l8960_overwrite2_len;
        payload.stage2_len = s5l8960_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = s5l8960_overwrite1;
        payload.over2 = s5l8960_overwrite2;
        payload.stage2 = s5l8960_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* S5L8960_PAYLOAD */
    
#if defined(T7000_PAYLOAD)
    if((client->devinfo.cpid) == 0x7000) {
        payload.over1_len = 0;
        payload.over2_len = t7000_overwrite2_len;
        payload.stage2_len = t7000_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = NULL;
        payload.over2 = t7000_overwrite2;
        payload.stage2 = t7000_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* T7000_PAYLOAD */
    
#if defined(T7001_PAYLOAD)
    if((client->devinfo.cpid) == 0x7001) {
        payload.over1_len = 0;
        payload.over2_len = t7001_overwrite2_len;
        payload.stage2_len = t7001_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = NULL;
        payload.over2 = t7001_overwrite2;
        payload.stage2 = t7001_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* T7001_PAYLOAD */
    
#if defined(S8000_PAYLOAD)
    if((client->devinfo.cpid) == 0x8000) {
        payload.over1_len = 0;
        payload.over2_len = s8000_overwrite2_len;
        payload.stage2_len = s8000_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = NULL;
        payload.over2 = s8000_overwrite2;
        payload.stage2 = s8000_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* S8000_PAYLOAD */
    
#if defined(S8003_PAYLOAD)
    if((client->devinfo.cpid) == 0x8003) {
        payload.over1_len = 0;
        payload.over2_len = s8003_overwrite2_len;
        payload.stage2_len = s8003_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = NULL;
        payload.over2 = s8003_overwrite2;
        payload.stage2 = s8003_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* S8003_PAYLOAD */
    
#if defined(S8001_PAYLOAD)
    if((client->devinfo.cpid) == 0x8001) {
        payload.over1_len = 0;
        payload.over2_len = s8001_overwrite2_len;
        payload.stage2_len = s8001_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = NULL;
        payload.over2 = s8001_overwrite2;
        payload.stage2 = s8001_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* S8001_PAYLOAD */
    
#if defined(T8010_PAYLOAD)
    if((client->devinfo.cpid) == 0x8010) {
        payload.over1_len = t8010_overwrite1_len;
        payload.over2_len = t8010_overwrite2_len;
        payload.stage2_len = t8010_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = t8010_overwrite1;
        payload.over2 = t8010_overwrite2;
        payload.stage2 = t8010_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* T8010_PAYLOAD */
    
#if defined(T8011_PAYLOAD)
    if((client->devinfo.cpid) == 0x8011) {
        payload.over1_len = t8011_overwrite1_len;
        payload.over2_len = t8011_overwrite2_len;
        payload.stage2_len = t8011_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = t8011_overwrite1;
        payload.over2 = t8011_overwrite2;
        payload.stage2 = t8011_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* T8010_PAYLOAD */
    
#if defined(T8015_PAYLOAD)
    if((client->devinfo.cpid) == 0x8015){
        payload.over1_len = t8015_overwrite1_len;
        payload.over2_len = t8015_overwrite2_len;
        payload.stage2_len = t8015_stage2_len;
        payload.pongoOS_len = pongoOS_len;
        payload.over1 = t8015_overwrite1;
        payload.over2 = t8015_overwrite2;
        payload.stage2 = t8015_stage2;
        payload.pongoOS = pongoOS;
    }
#endif /* T8015_PAYLOAD */
    
#endif /* BUILTIN_PAYLOAD */
    
//#if defined(KPF_FLAGS_PTR) && defined(BOOTARGS_STR_PTR)
//    checkrain_set_option(kpf_flags, checkrain_option_verbose_boot, 1);
//    bootargs = "rootdev=md0 -v";
//    DEBUGLOG("[%s] kpf_flags: %x", __FUNCTION__, kpf_flags);
//    DEBUGLOG("[%s] boot-args: %s", __FUNCTION__, bootargs);
//#endif
    
#if defined(T8015_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8015_PAYLOAD))
    if((client->devinfo.cpid == 0x8015)&&(devmode == 0x8015)){
        return checkra1n_t8010_t8015(client, payload); // A11 Bionic
    }
#endif
    
#if defined(T8011_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8011_PAYLOAD))
    if((client->devinfo.cpid == 0x8011)&&(devmode == 0x8011)){
        return checkra1n_t8010_t8015(client, payload); // A10X Fusion
    }
#endif
    
#if defined(T8010_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T8010_PAYLOAD))
    if((client->devinfo.cpid == 0x8010)&&(devmode == 0x8010)){
        return checkra1n_t8010_t8015(client, payload); // A10 Fusion
    }
#endif
    
#if defined(S8000_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8000_PAYLOAD))
    if((client->devinfo.cpid == 0x8000)&&(devmode == 0x8000)){
        return checkra1n_t7000_s8000(client, payload);  // A9
    }
#endif
    
 #if defined(S8000_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S8000_PAYLOAD))
    if((client->devinfo.cpid == 0x8003)&&(devmode == 0x8003)){
        return checkra1n_t7000_s8000(client, payload);  // A9
    }
#endif
  
#if defined(T7000_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(T7000_PAYLOAD))
    if((client->devinfo.cpid == 0x7000)&&(devmode == 0x7000)){
        return checkra1n_t7000_s8000(client, payload);  // A8
    }
#endif
    
#if defined(S5L8960_CODE) && (!defined(BUILTIN_PAYLOAD) || defined(S5L8960_PAYLOAD))
    if((client->devinfo.cpid == 0x8960)&&(devmode == 0x8960)){
        return checkra1n_s5l8960x(client, payload);    // A7
    }
#endif
    
    return 0;
}


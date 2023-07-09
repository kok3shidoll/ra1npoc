/*
 * ra1npoc - helper/recovery.c
 *
 * Copyright (c) 2021 - 2023 kok3shidoll
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
#include <common/log.h>
#include <common/common.h>

static long cpuTime;

RA1NPOC_STATIC_API static void interval(double sec)
{
    long now;
    double n_sec;
    double b_sec = (double)cpuTime / CLOCKS_PER_SEC;
    while(1)
    {
        now = clock();
        n_sec = (double)now / CLOCKS_PER_SEC;
        if ((n_sec-b_sec) > sec)
        {
            break;
        }
    }
    cpuTime = now;
}

RA1NPOC_STATIC_API static void prog(int sec)
{
    int i=0;
    int j=0;
    
    cpuTime = clock();
    
    for(i=0; i<sec; i++)
    {
        printf("[");
        for (j=0;j<i+1;j++)
        {
            printf("=");
        }
        for (;j<sec;j++)
        {
            printf(" ");
        }
        printf("] (%d/%d sec)\r", i+1, sec);
        fflush(stdout);
        interval(1);
    }
    printf("\n");
}

RA1NPOC_API int enterDFU(client_t *client)
{
    printf("#===============\n");
    printf("#\n");
    printf("# DFU helper\n");
    printf("#\n");
    printf("# Please follow the instructions below to operate the device\n");
    printf("# (1) Press <enter> key\n");
    printf("# (2) Press and hold Side and Volume down buttons together (4 sec)\n");
    printf("# (3) Release Side button But keep holding Volume down button (10 sec)\n");
    printf("#===============\n");
    printf("ready? it starts 3 seconds after press <enter> key.\n");
    printf("[STEP1] Press <enter> key >> ");
    getchar();
    printf("\n");
    cpuTime = clock();
    for(int i=0; i<3; i++)
    {
        printf("preparing... (STEP2 will start after %d seconds)\r", 3-i);
        fflush(stdout);
        interval(1);
    }
    
    printf("[STEP2] Press and hold Side and Volume down buttons together\n");
    int j=0;
    for(int i=0; i<4; i++)
    {
        if(i==1)
        {
            IOUSBSendReboot(client);
        }
        printf("[");
        for (j=0;j<i+1;j++)
        {
            printf("=");
        }
        for (;j<4;j++)
        {
            printf(" ");
        }
        printf("] (%d/%d sec)\r", i+1, 4);
        fflush(stdout);
        interval(1);
    }
    printf("\n");
    
    printf("[STEP3] Release Side button But keep holding Volume down button\n");
    prog(10);
    
    LOG("Reconnecting");
    if(IOUSBConnect(client, kDeviceDFUModeID, 5, 0, 10000))
    {
        ERR("DFU mode device not found");
        return -1;
    }
    
    LOG("Found DFU mode device");
    IOUSBClose(client);
    
    return 0;
}

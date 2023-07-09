/*
 * ra1npoc - main.c
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

#include <getopt.h>

#include <io/iousb.h>
#include <common/log.h>
#include <common/common.h>
#include <pongoterm.h>

#if defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
# define BUILD_STYLE "BRRA1NPOC15"
#endif

#if defined(RA1NPOC_MODE) && !defined(BAKERA1N_MODE)
# define BUILD_STYLE "RA1NPOC15"
#endif

#if !defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
# define BUILD_STYLE "BAKERA1N_LOADER"
#endif

#if !defined(RA1NPOC_MODE) && !defined(BAKERA1N_MODE)
# define BUILD_STYLE "NONE"
#endif

#if defined(DEVBUILD)
# define BUILD_TYPE "DEVELOPMENT"
#else
# define BUILD_TYPE "RELEASE"
#endif

#if defined(RA1NPOC_MODE)
# include <recovery.h>
# include <ra1npoc.h>

 // g
 bool           use_ra1npoc             = false;
 static bool    use_recovery            = false;
 static bool    checkra1n_mode          = false;
# if defined(BAKERA1N_MODE)
  bool          use_ra1npoc15           = false;
# endif
#endif

#if defined(BAKERA1N_MODE)
 static bool    bakera1n_mode           = false;
 bool           disable_cfprefsd_hook   = false;
 bool           use_lightweight_overlay = false;
#endif

#if defined(RA1NPOC_MODE) || defined(BAKERA1N_MODE)
# include <lz4_main.h>
# include "headers/Pongo_bin.h"

 static bool    use_early_exit          = false;
 static void*   customPongo             = NULL;
 static size_t  customPongoSize         = 0;

 static bool    use_yolodfu             = false;
 bool           use_safemode            = false;
 bool           use_verbose_boot        = false;
 char*          root_device             = NULL;
 char*          bootArgs                = NULL;
#endif

RA1NPOC_STATIC_API void usage(const char* arg0)
{
#if defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
    printf("Usage: %s [-rRp] [-hcyEsvnw] [-e <boot-args>] [-k <override_pongo>]\n", arg0);
#endif
    
#if defined(RA1NPOC_MODE) && !defined(BAKERA1N_MODE)
    printf("Usage: %s [-r] [-hcyEsv] [-e <boot-args>] [-k <override_pongo>]\n", arg0);
#endif
    
#if !defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
    printf("Usage: %s [-p] [-hyEsvnw] [-e <boot-args>] [-k <override_pongo>]\n", arg0);
#endif
    
    printf("  mode:\n");
#if defined(RA1NPOC_MODE)
    printf("\t-r, --ra1npoc\t\t\t: start with legacy ra1npoc mode\n");
#endif
    
#if defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
    printf("\t-R, --ra1npoc15\t\t\t: start with ra1npoc15 mode\n");
#endif
    
#if defined(BAKERA1N_MODE)
    printf("\t-p, --bakera1n\t\t\t: start with bakera1n mode\n");
#endif
    
    printf("\n  options:\n");
#if defined(RA1NPOC_MODE)
    printf("\t-c, --cleandfu\t\t\t: use clean dfu\n");
#endif
    
#if defined(RA1NPOC_MODE) || defined(BAKERA1N_MODE)
    printf("\t-y, --yolodfu\t\t\t: use download mode (yoloDFU)\n");
    printf("\t-E, --early-exit\t\t: exit after uploading Pongo\n");
    printf("\t-k, --override-pongo <path>\t: override Pongo image\n");
    printf("\t-e, --extra-bootargs <args>\t: replace bootargs\n");
    printf("\t-s, --safemode\t\t\t: enable safe mode\n");
    printf("\t-v, --verbose-boot\t\t: enable verbose boot\n");
#endif
    
#if defined(BAKERA1N_MODE)
    printf("\t-n, --no-cfprefsd-hook\t\t: disable cfprefsd hook\n");
    printf("\t-w, --lite-overlay\t\t: use lightweight overlay (beta)\n");
#endif
    
    printf("\n  help:\n");
    printf("\t-h, --help\t\t\t: show usage\n");
    return;
}

#if defined(RA1NPOC_MODE) || defined(BAKERA1N_MODE)
RA1NPOC_STATIC_API static int openFile(char *file, size_t *sz, void **buf)
{
    FILE *fd = fopen(file, "r");
    if (!fd)
    {
        ERR("error opening %s", file);
        return -1;
    }
    
    fseek(fd, 0, SEEK_END);
    *sz = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    *buf = malloc(*sz);
    if (!*buf)
    {
        ERR("error allocating file buffer");
        fclose(fd);
        return -1;
    }
    
    fread(*buf, *sz, 1, fd);
    fclose(fd);
    
    return 0;
}
#endif

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        usage(argv[0]);
        return -1;
    }
    
    int opt = 0;
    static struct option longopts[] = {
        // help
        { "help",               no_argument,       NULL, 'h' },
        
        // mode
#if defined(RA1NPOC_MODE)
        { "ra1npoc",            no_argument,       NULL, 'r' },
#endif
        
#if defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
        { "ra1npoc15",          no_argument,       NULL, 'R' },
#endif
        
#if defined(BAKERA1N_MODE)
        { "bakera1n",           no_argument,       NULL, 'p' },
        { "no-cfprefsd-hook",   no_argument,       NULL, 'n' },
        { "lite-overlay",       no_argument,       NULL, 'w' },
#endif
        
        // flags
#if defined(RA1NPOC_MODE)
        { "cleandfu",           no_argument,       NULL, 'c' },
#endif
        
#if defined(RA1NPOC_MODE) || defined(BAKERA1N_MODE)
        { "yolodfu",            no_argument,       NULL, 'y' },
        { "early-exit",         no_argument,       NULL, 'E' },
        { "override-pongo",     required_argument, NULL, 'k' },
        { "extra-bootargs",     required_argument, NULL, 'e' },
        { "safemode",           no_argument,       NULL, 's' },
        { "verbose-boot",       no_argument,       NULL, 'v' },
#endif
        { NULL, 0, NULL, 0 }
    };
    
#if defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
    const char* optStr = "hrRcEk:e:svpynw";
#endif
    
#if defined(RA1NPOC_MODE) && !defined(BAKERA1N_MODE)
    const char* optStr = "hrcEk:e:svy";
#endif
    
#if !defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
    const char* optStr = "heEk::svpynw";
#endif
    
#if !defined(RA1NPOC_MODE) && !defined(BAKERA1N_MODE)
    const char* optStr = "h";
#endif
    
    while ((opt = getopt_long(argc, argv, optStr, longopts, NULL)) > 0) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
                return 0;

#if defined(RA1NPOC_MODE)
            case 'r':
                use_ra1npoc = 1;
                checkra1n_mode = 1;
                LOG("selected: legacy ra1npoc mode");
                LOG("selected: checkra1n mode");
                break;
#endif
                
#if defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
            case 'R':
                use_ra1npoc15 = 1;
                LOG("selected: ra1npoc15 mode");
                break;
#endif
                
#if defined(BAKERA1N_MODE)
            case 'p':
                bakera1n_mode = 1;
                LOG("selected: bakera1n mode");
                break;
                
            case 'n':
                disable_cfprefsd_hook = 1;
                LOG("selected: disable cfprefsd hook");
                break;
                
            case 'w':
                use_lightweight_overlay = 1;
                LOG("selected: use lightweight overlay");
                break;
#endif
               
#if defined(RA1NPOC_MODE)
            case 'c':
                use_recovery = 1;
                break;
#endif

#if defined(RA1NPOC_MODE) || defined(BAKERA1N_MODE)
            case 'y':
                use_yolodfu = 1;
                LOG("selected: yolo bakera1n mode");
                break;
                
            case 'E':
                use_early_exit = 1;
                break;
                
            case 'k':
                if (optarg)
                {
                    if(openFile(optarg, &customPongoSize, &customPongo))
                        return -1;
                }
                else
                {
                    usage(argv[0]);
                    return -1;
                }
                break;
                
            case 'e':
                if (optarg)
                {
                    bootArgs = strdup(optarg);
                    LOG("set bootArgs: [%s]", bootArgs);
                }
                else
                {
                    usage(argv[0]);
                    return -1;
                }
                break;
                
            case 's':
                use_safemode = 1;
                break;
                
            case 'v':
                use_verbose_boot = 1;
                break;
#endif
                
            default:
                usage(argv[0]);
                return -1;
        }
    }
    
    LOG2("================================");
    LOG2("::");
    LOG2(":: ra1npoc15 for macOS/iOS/iPadOS");
    LOG2("::");
    LOG2(":: (c) 2020-2023 kok3shidoll (dora2ios)");
    LOG2("::");
    LOG2(":: BUILD_TAG         : ra1npoc15-%s", RPVERSION);
    LOG2(":: BUILD_STYLE       : %s_%s", BUILD_STYLE, BUILD_TYPE);
#if defined(RA1NPOC_MODE)
    LOG2(":: CHECKRA1N_VERSION : %s", "0.1337.2");
#endif
#if defined(BAKERA1N_MODE)
# if !defined(VERSION)
#  define VERSION "TEST"
# endif
    LOG2(":: BAKERA1N_VERSION  : %s", VERSION);
#endif
    LOG2("::");
    LOG2(":: ---- made by ----");
    LOG2(":: kok3shidoll (dora2ios)");
    LOG2(":: ---- thanks to ----");
    LOG2(":: checkra1n team");
#if defined(RA1NPOC_MODE)
    LOG2(":: P5-2005");
#endif
#if defined(BAKERA1N_MODE)
    LOG2(":: evelyneee, Nick Chan, opa334, Procursus Team, Siguza, tihmstar");
#endif
    LOG2("================================");
    LOG2("");
    
    // sanity checks
    int sanity = 1;
    
#if defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
    // mode conflict: ra1npoc_mode and ra1npoc15 mode
    if(use_ra1npoc && use_ra1npoc15)
    {
        ERR("Conflict: mode args");
        return -1;
    }
    // mode conflict: ra1npoc_mode and bakera1n_mode mode
    if(use_ra1npoc && bakera1n_mode)
    {
        ERR("Conflict: mode args");
        return -1;
    }
    
    // flag conflict: use_recovery and use_yolodfu mode
    if(use_recovery && use_yolodfu)
    {
        ERR("Conflict: flag args");
        return -1;
    }
    
    // flag conflict: bakera1n_mode and recovery mode
    if(use_recovery && bakera1n_mode)
    {
        ERR("Conflict: flag args");
        return -1;
    }
    
    if(!use_ra1npoc && !use_ra1npoc15 && !bakera1n_mode)
    {
        ERR("Operation mode is not selected");
        return -1;
    }
    else
    {
        // pass
        sanity = 0;
    }
#endif
    
#if defined(RA1NPOC_MODE) && !defined(BAKERA1N_MODE)
    // flag conflict: use_recovery and use_yolodfu mode
    if(use_recovery && use_yolodfu)
    {
        ERR("Conflict: flag args");
        return -1;
    }
    
    if(!use_ra1npoc)
    {
        ERR("Operation mode is not selected");
        return -1;
    }
    else
    {
        // pass
        sanity = 0;
    }
#endif
    
#if !defined(RA1NPOC_MODE) && defined(BAKERA1N_MODE)
    if(!bakera1n_mode)
    {
        ERR("Operation mode is not selected");
        return -1;
    }
    else
    {
        // pass
        sanity = 0;
    }
#endif
    
    if(sanity != 0)
    {
        ERR("Failed sanity check");
        return -1;
    }
    
#if defined(RA1NPOC_MODE) || defined(BAKERA1N_MODE)
    
    // Here, our device may be in the following modes
    // - DFU mode
    // - Recovery Mode
    // - yoloDFU mode
    // - pongoOS mode
    // - unknown
    
    client_t client = {};
    int waitsec = 5;
    
    __unused int check_recovery_device = 0;
    int check_dfu_device = 0;
    int check_pongo_device = 0;
    
    int found_dfu_device = 0;
    int found_yolodfu_device = 0;
    int found_pongo_device = 0;
    
#if defined(RA1NPOC_MODE)
    // check if we need to see if the Recovery device (only RA1NPOC_MODE)

    if(use_recovery) check_recovery_device = 1;
    
    if(check_recovery_device)
    {
        // Recovery Mode -> DFU mode
        waitsec = 5;
        LOG("Waiting for recovery mode device (%d sec)", waitsec);
        if(IOUSBConnect(&client, kDeviceRecovery2ModeID, waitsec, 0, 10000))
        {
            ERR("Recovery mode device not found");
            return -1;
        }
        LOG("Found: recovery mode device");
        DEVLOG("%04x %02x", client.cpid, client.cprv);
        
        if(enterDFU(&client))
        {
            return -1;
        }
        check_recovery_device = 0;
        found_dfu_device = 1;
    }
#endif
    
    // Here, our device may be in the following modes
    // - DFU mode
    // - yoloDFU mode
    // - pongoOS mode
    // - unknown
    
    // check if we need to see if the DFU device
#if defined(RA1NPOC_MODE)
    if(use_ra1npoc) check_dfu_device = 1;
#if defined(BAKERA1N_MODE)
    if(use_ra1npoc15) check_dfu_device = 1;
#endif /* RA1NPOC_MODE && BAKERA1N_MODE */
#endif /* RA1NPOC_MODE */
    
#if defined(BAKERA1N_MODE)
    if(bakera1n_mode && use_yolodfu)
    {
        check_dfu_device = 1;
    }
    else
    {
        if(!check_dfu_device) check_pongo_device = 1;
    }
#endif /* BAKERA1N_MODE */
    
    if(check_dfu_device)
    {
        // - DFU mode
        // - yoloDFU mode
        waitsec = 5;
        LOG("Waiting for DFU mode device (%d sec)", waitsec);
        if(IOUSBConnect(&client, kDeviceDFUModeID, waitsec, 0, 10000))
        {
            ERR("DFU mode device not found");
            return -1;
        }
        
        LOG("Found: DFU mode device");
        DEVLOG("%04x %02x", client.cpid, client.cprv);
        found_dfu_device = 1;
        
        if(client.devmode & kDeviceYoloDFUMode)
        {
            // - yoloDFU mode
            LOG("Found: yoloDFU mode device");
            found_dfu_device = 0;
            found_yolodfu_device = 1;
        }
    }
    
    // If the device is in DFU mode, run checkm8 exploit
    if(found_dfu_device)
    {
#if defined(RA1NPOC_MODE)
        // DFU mode -> yoloDFU mode
        // based checkra1n 0.1337
        if(ra1npoc15(&client, client.cpid))
        {
            ERR("Failed to exeute checkm8 exploit");
            return -1;
        }
        
        // check yoloDFU mode
        waitsec = 20;
        LOG("Waiting for DFU mode device (%d sec)", waitsec);
        if(IOUSBConnect(&client, kDeviceDFUModeID, waitsec, 0, 10000))
        {
            ERR("DFU mode device not found");
            return -1;
        }
        
        if(!(client.devmode & kDeviceYoloDFUMode))
        {
            ERR("Failed to enter yoloDFU mode");
            IOUSBClose(&client);
            return -1;
        }
        // - yoloDFU mode
        LOG("Found: yoloDFU mode device");
        found_dfu_device = 0;
        found_yolodfu_device = 1;
#else
        ERR("This build does not have ra1npoc mode");
        ERR("please put the device in yoloDFU mode or pongoOS mode and run it again.");
        return -1;
#endif
    }
    
    // Here, our device may be in the following modes
    // - yoloDFU mode
    // - pongoOS mode
    // - unknown
    
    // If the device is in yoloDFU mode, send pongoOS.
    if(found_yolodfu_device)
    {
        void* Pongo_LZ4 = NULL;
        size_t Pongo_LZ4_len = 0;
        
        if((customPongo == NULL) || (customPongoSize == 0))
        {
            DEVLOG("Compressing pongoOS");
            if(lz4CompressAndAddShellcode(Pongo_bin, Pongo_bin_len, &Pongo_LZ4, &Pongo_LZ4_len))
            {
                ERR("failed to compress pongoOS");
                IOUSBClose(&client);
                return -1;
            }
        }
        else
        {
            DEVLOG("Compressing custom pongoOS");
            if(lz4CompressAndAddShellcode(customPongo, customPongoSize, &Pongo_LZ4, &Pongo_LZ4_len))
            {
                ERR("failed to compress custom pongoOS");
                IOUSBClose(&client);
                return -1;
            }
        }
        
        if(!Pongo_LZ4)
        {
            ERR("failed to generate pongo image");
            IOUSBClose(&client);
            return -1;
        }
        
        if(Pongo_LZ4_len > 0x40000)
        {
            ERR("pongo image is too large");
            IOUSBClose(&client);
            return -1;
        }
        
        LOG("Sending pongoOS");
        if(sendPongo(&client, Pongo_LZ4, Pongo_LZ4_len))
        {
            ERR("failed to send pongoOS");
            return -1;
        }
        waitsec = 20;
        found_yolodfu_device = 0;
        check_pongo_device = 1;
    }
    
    // Here, our device may be in the following modes
    // - pongoOS mode
    // - unknown
    
    if(check_pongo_device)
    {
        LOG("Waiting for pongo mode device (%d sec)", waitsec);
        if(IOUSBConnect(&client, kDevicePongoModeID, waitsec, 0, 10000))
        {
            ERR("Pongo mode device not found");
            return -1;
        }
        // - pongoOS mode
        LOG("Found: pongo mode device");
        DEVLOG("Found %04x %02x", client.cpid, client.cprv);
        check_pongo_device = 0;
        found_pongo_device = 1;
        
        // set next mode
#if defined(RA1NPOC_MODE)
        if(use_ra1npoc)
        {
            checkra1n_mode = 1;
        }
# if defined(BAKERA1N_MODE)
        if(use_ra1npoc15)
        {
            bakera1n_mode = 1;
        }
# endif
#endif
        if(use_early_exit)
        {
#if defined(RA1NPOC_MODE)
            checkra1n_mode = 0;
#endif
#if defined(BAKERA1N_MODE)
            bakera1n_mode = 0;
#endif
        }
    }
    
    if(found_pongo_device)
    {
#if defined(BAKERA1N_MODE)
        if(bakera1n_mode)
        {
            if(!pongoTerm(&client))
            {
                IOUSBClose(&client);
                return -1;
            }
            LOG("booted?");
            return 0;
        }
#endif
        
#if defined(RA1NPOC_MODE)
        if(checkra1n_mode)
        {
            if(!oldPongoTerm(&client))
            {
                IOUSBClose(&client);
                return -1;
            }
            
            LOG("booted?");
            return 0;
        }
#endif
    }
#endif
    
    return 0;
}

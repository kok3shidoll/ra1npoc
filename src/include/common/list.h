/*
 * list.h
 *
 * Copyright (c) 2021 dora2ios
 *
 * List of supported devices.
 * Currently, source code of payload used with checkra1n is not publicly available.
 * However, checkm8 exploit itself is open source and can be clonedby analyzing
 * USB packets sent by checkra1n.
 *
 */

#ifndef LIST_H
#define LIST_H

/*
 * S5L8960 (Apple A7)
 * The S5L8960 supports only exploit code.
 * Some of the necessary payloads will need to be prepared by yourself.
 */
#define S5L8960_CODE
//#define S5L8960_PAYLOAD


/*
 * T7000 (Apple A8) // tested on iPhone7,2
 * T8015 is fully supported.
 */
#define T7000_CODE
#define T7000_PAYLOAD


/*
 * T7001 (Apple A8X)
 * T7001 is not supported.
 */
//#define T7001_CODE
//#define T7001_PAYLOAD


/*
 * S8000 (Apple A9)
 * The S8000 supports only exploit code.
 * Some of the necessary payloads will need to be prepared by yourself.
 */
#define S8000_CODE
//#define S8000_PAYLOAD
//#define S8001_CODE
//#define S8001_PAYLOAD
//#define S8003_CODE
//#define S8003_PAYLOAD


/*
 * T8010 (Apple A10 Fusion)
 * T8010 is fully supported.
 */
#define T8010_CODE
#define T8010_PAYLOAD


/*
 * T8011 (Apple A10X Fusion)
 * T7001 is not supported.
 */
//#define T8011_CODE
//#define T8011_PAYLOAD


/*
 * T8015 (Apple A11 Bionic)
 * T8015 is fully supported.
 */
#define T8015_CODE
#define T8015_PAYLOAD


#endif

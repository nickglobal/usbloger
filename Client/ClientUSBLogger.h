/*
 * ClientUSBLogger.h
 *
 *  Created on: Oct 18, 2011
 *      Author: Igor
 */

#ifndef CLIENTUSBLOGGER_H_
#define CLIENTUSBLOGGER_H_

#define LEN_STR (255)
#define LEN_BUF (1024)
#define MRK_BEGIN (0xBE)
#define MRK_END (0xED)
#define FIELD_NUM (6)

typedef  struct  DevUsb_s {
    char devnode[LEN_STR];
    char serial[LEN_STR];
    char vendorId[LEN_STR];
    char productId[LEN_STR];
    char action[LEN_STR];
} DevUsb;



#endif /* CLIENTUSBLOGGER_H_ */

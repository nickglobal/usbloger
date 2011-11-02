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

enum {
    TIME_REC,
    HOST_REC,
    SERIAL_REC,
    ACTION_REC,
    VENDOR_NAME_REC,
    VENDOR_ID_REC,
    PRODUCT_NAME,
    PRODUCT_ID,
    NUMB_REC
};

typedef struct MsgRecord_s {
    char name[LEN_STR];
    char value[LEN_STR];
}MsgRecord;


typedef struct DevMsg_s {
    char devnode[LEN_STR];
    MsgRecord record[NUMB_REC];
} DevMsg;



#endif /* CLIENTUSBLOGGER_H_ */

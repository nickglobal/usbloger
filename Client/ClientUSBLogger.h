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
#define WRP_BEGIN (0xA5)
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
    UPTIME_SYS,
    UPTIME_CLIENT,
    NUMB_REC
};

typedef struct Header_s {
    int count;
    long long fileSize;
}Header;

typedef struct MsgRecord_s {
    char name[LEN_STR];
    char value[LEN_STR];
}MsgRecord;

typedef struct  __attribute__((packed)) Wraper_s{
    unsigned char markBegin;
    unsigned short lenBuf;
}Wrapper;

typedef struct __attribute__((packed)) WraperMsg_s {
    Wrapper wrp;
    char rawBuf[LEN_BUF] ;
}WraperMsg ;

typedef struct DevMsg_s {
    char devnode[LEN_STR];
    MsgRecord record[NUMB_REC];
}DevMsg;

void LogUdevEvents (void);
void delete_buf_msg(void);
void send_msg_to_server(char* serverIP);
unsigned int get_msg_crc(void);

#endif /* CLIENTUSBLOGGER_H_ */

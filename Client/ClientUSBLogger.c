/*
 * ClienUSBLogger.c
 *
 *  Created on: Oct 12, 2011
 *      Author:  TI x0169005
 */
#include "common.h"
#include "ClientUSBLogger.h"
#include "ListDev.h"
#include "SocketUDP.h"
#include <libudev.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>
#include </usr/include/asm-generic/errno-base.h>
#include <syslog.h>
#include "crc32.h"
#include <time.h>
#include <sys/times.h>

#define PATH2LOG "/var/log/.cul.r"
#define PATH2BUF "/var/log/.cul.b"

static long startTime;

static int get_record_str(MsgRecord* dest,const char* name, const char* value)
{
    int status = EXIT_FAILURE;

    if(NULL != name)
    {
        status = EXIT_SUCCESS;
        strncpy(dest->name, name, LEN_STR);
    }
    else
    {
        dest->name[0] = 0;
        status = EXIT_FAILURE;
    }

    if(NULL != value)
    {
        status = EXIT_SUCCESS;
        strncpy(dest->value, value, LEN_STR);
    }
    else
    {
        dest->value[0] = 0;
        status = EXIT_FAILURE;
    }

    return status;
}

static void add_record2buff(char *buff, MsgRecord *rec, unsigned short *len)
{
    //add name
    *buff = strlen(rec->name);
    *len += *buff + 2;
    strcpy(buff+1, rec->name);
    //add value
    buff +=  ((*buff) + 2);
    *buff = strlen(rec->value);
    *len += *buff + 2;
    strcpy(buff+1, rec->value);
}

static long long get_filesize(const char *path)
{
    FILE *pFile;
    long long fileSize = 0;

    pFile = fopen (path, "rb");
    if(NULL == pFile)
    {
        syslog(LOG_ERR | LOG_USER, "Fail to open(r+b) Buf file");
    }

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    fileSize = ftell (pFile);

    fclose(pFile);

    return fileSize;
}

static void erase_buf_file(void)
{
    FILE *pFileBuf;

    pFileBuf = fopen (PATH2BUF,"w");
    if(NULL == pFileBuf)
    {
        syslog(LOG_ERR | LOG_USER, "Fail to open(w) Buf file");
    }

    fclose(pFileBuf);
}

int get_msg_buf_len(unsigned char *pBuf, int *pLen)
{
    Wrapper wrp;
    FILE *pFileBuf;
    int status = EXIT_FAILURE;
    long long fileSize = 0;

    *pLen = 0;

    fileSize = get_filesize(PATH2BUF);

    if(0 != fileSize)
    {
        do{
            pFileBuf = fopen (PATH2BUF,"rb");
            if(NULL == pFileBuf)
            {
                syslog(LOG_ERR | LOG_USER, "Fail to open(rb) Buf file");
                break;
            }

            if(sizeof(wrp) != fread(&wrp, 1, sizeof(wrp), pFileBuf))
            {
                if(0 != feof(pFileBuf))
                {
                    syslog(LOG_ERR | LOG_USER, "In the Buf file EOF is reached.");
                }
                else
                {
                    syslog(LOG_ERR | LOG_USER, "Fail to read wrapper of msg string from Buf file.");
                }
                break;
            }

            if(WRP_BEGIN != wrp.markBegin)
            {
                syslog(LOG_ERR | LOG_USER, "Wrong marker begin wrapper was obtained");
                break;
            }

            if(wrp.lenBuf != fread(pBuf, 1, wrp.lenBuf, pFileBuf))
            {
                if(0 != feof(pFileBuf))
                {
                    syslog(LOG_ERR | LOG_USER, "Error to read msg from Buf file, unexpected EOF reached.");
                }
                else
                {
                    syslog(LOG_ERR | LOG_USER, "Fail to read msg from Buf file.");
                }
                break;
            }

            *pLen = wrp.lenBuf;
            status = EXIT_SUCCESS;
        }
        while(0);

        fclose(pFileBuf);

        if (EXIT_FAILURE == status)
        {
            erase_buf_file();
        }

    }//    if(0 != fileSize)

    return status;
}

void send_msg_to_server(char* serverIP)
{
    unsigned char buf[LEN_BUF];
    int lenBuf;

    if (EXIT_SUCCESS == get_msg_buf_len(buf, &lenBuf))
    {
        if (EXIT_SUCCESS != send_buffer_to_server(serverIP, PORT, buf, lenBuf) )
        {
            syslog(LOG_ERR | LOG_USER, "Fail to send message through UDP");
        }
    }
}

unsigned int get_msg_crc(void)
{
    unsigned char buf[LEN_BUF];
    int lenBuf;
    unsigned int crc32_val = 0;

    if (EXIT_SUCCESS == get_msg_buf_len(buf, &lenBuf))
    {
        crc32_val = crc32(buf, lenBuf);
    }

    return crc32_val;
}



void delete_buf_msg(void)
{
    FILE * pFileBuf;
    long long  fileSize;
    char *pBuf, *pBufOffset;
    Wrapper *wrp;
    int erase = 0;

    fileSize = get_filesize(PATH2BUF);

    if(0 != fileSize)
    {
        do
        {
            pFileBuf = fopen ( PATH2BUF , "r+b" );
            if(NULL == pFileBuf)
            {
                syslog(LOG_ERR | LOG_USER, "Fail to open(r+b) Buf file");
                break;
            }

            // allocate memory to contain the whole file:
            pBuf = (char*) malloc (fileSize);
            if (pBuf == NULL)
            {
                syslog(LOG_ERR | LOG_USER, "Fail to malloc() buff. Size=%lld", fileSize);
                erase = 1;
                break;
            }

            // copy the file into the buffer:
            if (fileSize != fread (pBuf,1,fileSize,pFileBuf))
            {
                syslog(LOG_ERR | LOG_USER, "Fail copy Buf to file");
                erase = 1;
                break;
            }

            wrp = (Wrapper *)pBuf;

            if (wrp->lenBuf > fileSize)
            {
                syslog(LOG_ERR | LOG_USER, "Corrupted msg in the Buf file");
                erase = 1;
                break;
            }

            fclose (pFileBuf);

            erase_buf_file();

            pFileBuf = fopen ( PATH2BUF , "r+b" );
            if(NULL == pFileBuf)
            {
                syslog(LOG_ERR | LOG_USER, "Fail to open(r+b) Buf file");
                break;
            }

            pBufOffset = pBuf + wrp->lenBuf + sizeof(Wrapper);
            fileSize -= (wrp->lenBuf + sizeof(Wrapper));

            if ( fileSize != fwrite(pBufOffset, 1, fileSize, pFileBuf) )
            {
                syslog(LOG_ERR | LOG_USER, "Fail write Buf to file");
                erase = 1;
                break;
            }

            free (pBuf);
        }
        while(0);

        fclose (pFileBuf);

        if (erase)
        {
            erase_buf_file();
        }
    }//if(0 != fileSize)
}

static void print_log(DevMsg *dev, int numRec)
{
    FILE *pFileLog;
    int i;

    pFileLog = fopen (PATH2LOG,"a+");
    if(NULL == pFileLog)
    {
        syslog(LOG_ERR | LOG_USER, "Fail to open(a+) Log file");
        DPRINT("Fail to open(a+) Log file");
    }
    else
    {
        for(i=0; i<numRec; i++)
        {
            fputs(dev->record[i].name, pFileLog);
            fputs(":", pFileLog);
            fputs(dev->record[i].value, pFileLog);
            fputs(" ", pFileLog);
        }
        fputs("\n", pFileLog);
    }

    fclose(pFileLog);
}

static void dd_hh_mm_ss(long seccount, char* time_str)
{
    unsigned int days, hours, mins, secs, offset;

    secs = seccount % 60;
    seccount /= 60;
    mins = seccount % 60;
    seccount /= 60;
    hours = seccount % 24;
    days = seccount / 24;
    offset = sprintf(time_str,"%03d",days);
    time_str[offset++]=':';
    offset += sprintf(time_str + offset,"%02d",hours);
    time_str[offset++]=':';
    offset += sprintf(time_str + offset,"%02d",mins);
    time_str[offset++]=':';
    sprintf(time_str + offset,"%02d",secs);
}

static void uptime_log(DevMsg *devMsg)
{
    char upTimeSys[LEN_STR] = "000:00:00:00";
    char upTimeClient[LEN_STR] = "000:00:00:00";
    struct timespec uptimesys;

    struct tms tm;
    long runtime;
    static int singletone = 0;

    if (!singletone)
    {
        singletone = 1;
        startTime = times(&tm);
    }

    do
    {
        if ( EXIT_FAILURE == clock_gettime(CLOCK_MONOTONIC, &uptimesys) )
        {
            syslog(LOG_ERR | LOG_USER, "Fail to open(a+) Buf file");
            DPRINT("Fail to open(a+) Buf file");
            break;
        }

        dd_hh_mm_ss((long)uptimesys.tv_sec, upTimeSys);

        runtime = times(&tm);
        runtime = (runtime - startTime) / sysconf(_SC_CLK_TCK);
        dd_hh_mm_ss(runtime, upTimeClient);

        get_record_str(&devMsg->record[ACTION_REC], "Action", "start logger");
        get_record_str(&devMsg->record[SERIAL_REC], "Serial", "");
        get_record_str(&devMsg->record[VENDOR_NAME_REC], "VendorName", "");
        get_record_str(&devMsg->record[VENDOR_ID_REC], "VendorID", "");
        get_record_str(&devMsg->record[PRODUCT_NAME], "ProductName", "");
        get_record_str(&devMsg->record[PRODUCT_ID], "ProductID", "");
        get_record_str(&devMsg->record[UPTIME_SYS], "UpTimeSys DDD:HH:MM:SS", upTimeSys);
        get_record_str(&devMsg->record[UPTIME_CLIENT], "UpTimeClient DDD:HH:MM:SS", upTimeClient);
    }
    while(0);
}

static void write_log(struct udev_device *dev)
{
    char curTime[LEN_STR] = {0};
    char curHost[LEN_STR] = {0};
    DevMsg devMsg;
    time_t t;
    int strLen;
    WraperMsg msg;
    FILE *pFileBuf;

    do
    {
        pFileBuf = fopen (PATH2BUF,"a+");
        if(NULL == pFileBuf)
        {
            syslog(LOG_ERR | LOG_USER, "Fail to open(a+) Buf file");
            DPRINT("Fail to open(a+) Buf file");
            break;
        }

        t = time(0);
        strncpy(curTime, ctime( &t ), LEN_STR);
        strLen = strlen(curTime);
        curTime[strLen-1] = 0;//flush \n in the time format
        gethostname( curHost, LEN_STR);

        get_record_str(&devMsg.record[TIME_REC], "Time", curTime);
        get_record_str(&devMsg.record[HOST_REC], "Host", curHost);

        if(NULL == dev)
        {
            uptime_log(&devMsg);
        }
        else
        {
            uptime_log(&devMsg);

            strcpy(devMsg.devnode, udev_device_get_devnode(dev));

            if ( EXIT_FAILURE == get_record_str(&devMsg.record[ACTION_REC], "Action", udev_device_get_action(dev)) )
            {
                get_record_str(&devMsg.record[ACTION_REC], "Action", "inserted");
            }

            get_record_str(&devMsg.record[SERIAL_REC], "Serial", udev_device_get_property_value(dev, "ID_SERIAL"));
            get_record_str(&devMsg.record[VENDOR_NAME_REC], "VendorName", udev_device_get_property_value(dev, "ID_VENDOR"));
            get_record_str(&devMsg.record[VENDOR_ID_REC], "VendorID", udev_device_get_property_value(dev, "ID_VENDOR_ID"));
            get_record_str(&devMsg.record[PRODUCT_NAME], "ProductName", udev_device_get_property_value(dev, "ID_MODEL"));
            get_record_str(&devMsg.record[PRODUCT_ID], "ProductID", udev_device_get_property_value(dev, "ID_MODEL_ID"));

            if(0 == strcmp(devMsg.record[ACTION_REC].value, "change"))
            {
                break;//skip all udev events with action "change"
            }

        }//else if(NULL == dev)

        syslog(LOG_ERR | LOG_USER, "New USB Event");
        syslog(LOG_ERR | LOG_USER, "Device Node Path= %s", devMsg.devnode);
        syslog(LOG_ERR | LOG_USER, "  Time= %s : %s", devMsg.record[TIME_REC].name, devMsg.record[TIME_REC].value);
        syslog(LOG_ERR | LOG_USER, "  Host= %s : %s", devMsg.record[HOST_REC].name, devMsg.record[HOST_REC].value);
        syslog(LOG_ERR | LOG_USER, "  Action= %s : %s", devMsg.record[ACTION_REC].name, devMsg.record[ACTION_REC].value);
        syslog(LOG_ERR | LOG_USER, "  Serial= %s : %s",devMsg.record[SERIAL_REC].name, devMsg.record[SERIAL_REC].value);
        syslog(LOG_ERR | LOG_USER, "  VendorName= %s : %s", devMsg.record[VENDOR_NAME_REC].name, devMsg.record[VENDOR_NAME_REC].value);
        syslog(LOG_ERR | LOG_USER, "  VendorID= %s : %s", devMsg.record[VENDOR_ID_REC].name, devMsg.record[VENDOR_ID_REC].value);
        syslog(LOG_ERR | LOG_USER, "  ProductName= %s : %s", devMsg.record[PRODUCT_NAME].name, devMsg.record[PRODUCT_NAME].value);
        syslog(LOG_ERR | LOG_USER, "  ProductID= %s : %s", devMsg.record[PRODUCT_ID].name, devMsg.record[PRODUCT_ID].value);
        syslog(LOG_ERR | LOG_USER, "  UpTimeSys= %s : %s", devMsg.record[UPTIME_SYS].name, devMsg.record[UPTIME_SYS].value);
        syslog(LOG_ERR | LOG_USER, "  UpTimeClient= %s : %s", devMsg.record[UPTIME_CLIENT].name, devMsg.record[UPTIME_CLIENT].value);
        DPRINT("New USB Event\n");
        DPRINT("Device Node Path= %s\n", devMsg.devnode);
        DPRINT("  Time= %s : %s\n", devMsg.record[TIME_REC].name, devMsg.record[TIME_REC].value);
        DPRINT("  Host= %s : %s\n", devMsg.record[HOST_REC].name, devMsg.record[HOST_REC].value);
        DPRINT("  Action= %s : %s\n", devMsg.record[ACTION_REC].name, devMsg.record[ACTION_REC].value);
        DPRINT("  Serial= %s : %s\n",devMsg.record[SERIAL_REC].name, devMsg.record[SERIAL_REC].value);
        DPRINT("  VendorName= %s : %s\n", devMsg.record[VENDOR_NAME_REC].name, devMsg.record[VENDOR_NAME_REC].value);
        DPRINT("  VendorID= %s : %s\n", devMsg.record[VENDOR_ID_REC].name, devMsg.record[VENDOR_ID_REC].value);
        DPRINT("  ProductName= %s : %s\n", devMsg.record[PRODUCT_NAME].name, devMsg.record[PRODUCT_NAME].value);
        DPRINT("  ProductID= %s : %s\n", devMsg.record[PRODUCT_ID].name, devMsg.record[PRODUCT_ID].value);
        DPRINT("  UpTimeSys= %s : %s\n", devMsg.record[UPTIME_SYS].name, devMsg.record[UPTIME_SYS].value);
        DPRINT("  UpTimeClient= %s : %s\n", devMsg.record[UPTIME_CLIENT].name, devMsg.record[UPTIME_CLIENT].value);

        memset(&msg, 0, sizeof(msg));
        msg.wrp.markBegin = WRP_BEGIN;
        msg.rawBuf[msg.wrp.lenBuf++] = MRK_BEGIN;
        msg.rawBuf[msg.wrp.lenBuf++] = NUMB_REC;

        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[TIME_REC], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[HOST_REC], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[SERIAL_REC], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[ACTION_REC], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[VENDOR_NAME_REC], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[VENDOR_ID_REC], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[PRODUCT_NAME], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[PRODUCT_ID], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[UPTIME_SYS], &msg.wrp.lenBuf);
        add_record2buff(&msg.rawBuf[msg.wrp.lenBuf], &devMsg.record[UPTIME_CLIENT], &msg.wrp.lenBuf);

        msg.rawBuf[msg.wrp.lenBuf++] = MRK_END;

        fwrite(&msg, 1, msg.wrp.lenBuf + sizeof(Wrapper), pFileBuf);

        print_log(&devMsg, NUMB_REC);
    }
    while(0);

    fclose (pFileBuf);
}

void LogUdevEvents (void)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *list_block, *dev_list_entry;
    struct udev_device *dev, *devBlock;

    static struct udev_monitor *mon_block;
    static int fd;

    fd_set fds;
    struct timeval timeout;
    int ret;
    static int singletone = 0;
    const char *path;
    char devnode[LEN_STR];

    if (!singletone)
    {
        singletone = 1;

        /* Create the udev object */
        udev = udev_new();
        if (!udev)
        {
            DPRINT("Can't create udev\n");
            exit(1);
        }

        /* Set up a monitor to monitor disk list_block */
        mon_block = udev_monitor_new_from_netlink(udev, "udev");
        udev_monitor_filter_add_match_subsystem_devtype(mon_block, "block", "disk");
//        udev_monitor_filter_add_match_subsystem_devtype(mon_block, "usb", "usb_device");
//        udev_monitor_filter_add_match_subsystem_devtype(mon_block, "scsi", "scsi_device");
//        udev_monitor_filter_add_match_subsystem_devtype(mon_block, "bdi", NULL);
        udev_monitor_enable_receiving(mon_block);
        /* Get the file descriptor (fd) for the monitor.
           This fd will get passed to select() */
        fd = udev_monitor_get_fd(mon_block);

        /* Create a list of the list_block in the 'block' subsystem. */
        enumerate = udev_enumerate_new(udev);
        udev_enumerate_add_match_subsystem(enumerate, "block");
        udev_enumerate_scan_devices(enumerate);
        list_block = udev_enumerate_get_list_entry(enumerate);

        write_log(0);//Send to server startup time

        udev_list_entry_foreach(dev_list_entry, list_block)
        {

            path = udev_list_entry_get_name(dev_list_entry);
            dev = udev_device_new_from_syspath(udev, path);

            devBlock = udev_device_get_parent_with_subsystem_devtype(
                   dev,
                   "block",
                   "disk");
            if (devBlock)
            {
                if ( strcmp(devnode, udev_device_get_devnode(devBlock)) != 0 )
                {
                  write_log(devBlock);
                  strcpy(devnode, udev_device_get_devnode(devBlock));
                }
            }

            udev_device_unref(dev);
        }

        /* Free the enumerator object */
        udev_enumerate_unref(enumerate);
    }//if (!singletone)
    else
    {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &timeout);

        /* Check if our file descriptor has received data. */
        if (ret > 0 && FD_ISSET(fd, &fds))
        {
            /* Make the call to receive the device.
               select() ensured that this will not block. */

            devBlock = udev_monitor_receive_device(mon_block);
            if (devBlock)
            {
                write_log(devBlock);
            }

            udev_device_unref(devBlock);
        }
        //DPRINT(".");
        fflush(stdout);
    }//else
}

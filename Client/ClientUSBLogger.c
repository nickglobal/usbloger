/*
 * ClienUSBLogger.c
 *
 *  Created on: Oct 12, 2011
 *      Author: Igor.Medvyedyev
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

char HostName[LEN_STR];

int get_record_str(MsgRecord* dest,const char* name, const char* value)
{
    int status = EINVAL;

    if(NULL != name)
    {
        status = 0;
        strncpy(dest->name, name, LEN_STR);
    }
    else
    {
        dest->name[0] = 0;
        status = EINVAL;
    }

    if(NULL != value)
    {
        status = 0;
        strncpy(dest->value, value, LEN_STR);
    }
    else
    {
        dest->value[0] = 0;
        status = EINVAL;
    }

    return status;
}

void add_record2buff(char *buff, MsgRecord *rec, int *len)
{
    *buff = strlen(rec->name);
    *len += *buff + 2;

    //DPRINT("Len1: %d\n",*buff + 2);
    strcpy(buff+1, rec->name);
    //DPRINT("Str1: %s\n",buff+1);
    //DPRINT("buf1 adr: %d\n",buff+1);

    buff +=  ((*buff) + 2);
    *buff = strlen(rec->value);
    *len += *buff + 2;
    //DPRINT("Len2: %d\n",*buff + 2);
    strcpy(buff+1, rec->value);

    //DPRINT("Str2: %s\n",buff+1);
    //DPRINT("buf2 adr: %d\n",buff+1);
    //DPRINT("Index: %d\n",*len);

}

void write_log(struct udev_device *dev)
{
    char curTime[LEN_STR] = {0};
    char curHost[LEN_STR] = {0};
    DevMsg devMsg;
    time_t t;

    int strLen;
    char rawBuf[LEN_BUF];
    int lenBuf;
    FILE * pFile;
    //int i;

    pFile = fopen ("ClientReport.txt","a+");

    t = time(0);
    strncpy(curTime, ctime( &t ), LEN_STR);
    strLen = strlen(curTime);
    curTime[strLen-1] = 0;//flush \n in the time format
    gethostname( curHost, LEN_STR);

    strcpy(devMsg.devnode, udev_device_get_devnode(dev));
    get_record_str(&devMsg.record[ACTION_REC], "Action", udev_device_get_action(dev));
    if ( EINVAL == get_record_str(&devMsg.record[ACTION_REC], "Action", udev_device_get_action(dev)) )
    {
        get_record_str(&devMsg.record[ACTION_REC], "Action", "inserted");
    }

    get_record_str(&devMsg.record[TIME_REC], "Time", curTime);
    get_record_str(&devMsg.record[HOST_REC], "Host", curHost);
    get_record_str(&devMsg.record[SERIAL_REC], "Serial", udev_device_get_property_value(dev, "ID_SERIAL"));
    get_record_str(&devMsg.record[VENDOR_NAME_REC], "VendorName", udev_device_get_property_value(dev, "ID_VENDOR"));
    get_record_str(&devMsg.record[VENDOR_ID_REC], "VendorID", udev_device_get_property_value(dev, "ID_VENDOR_ID"));
    get_record_str(&devMsg.record[PRODUCT_NAME], "ProductName", udev_device_get_property_value(dev, "ID_MODEL"));
    get_record_str(&devMsg.record[PRODUCT_ID], "ProductID", udev_device_get_property_value(dev, "ID_MODEL_ID"));

    DPRINT("\n");
    DPRINT("  Time= %s : %s\n", devMsg.record[TIME_REC].name, devMsg.record[TIME_REC].value);
    DPRINT("  Host= %s : %s\n", devMsg.record[HOST_REC].name, devMsg.record[HOST_REC].value);
    DPRINT("  Action= %s : %s\n", devMsg.record[ACTION_REC].name, devMsg.record[ACTION_REC].value);
    DPRINT("  Device Node Path= %s\n", devMsg.devnode);
    DPRINT("  Serial= %s : %s\n",devMsg.record[SERIAL_REC].name, devMsg.record[SERIAL_REC].value);
    DPRINT("  VendorName= %s : %s\n", devMsg.record[VENDOR_NAME_REC].name, devMsg.record[VENDOR_NAME_REC].value);
    DPRINT("  VendorID= %s : %s\n", devMsg.record[VENDOR_ID_REC].name, devMsg.record[VENDOR_ID_REC].value);
    DPRINT("  ProductName= %s : %s\n", devMsg.record[PRODUCT_NAME].name, devMsg.record[PRODUCT_NAME].value);
    DPRINT("  ProductID= %s : %s\n", devMsg.record[PRODUCT_ID].name, devMsg.record[PRODUCT_ID].value);

    lenBuf = 0;
    memset(rawBuf, 0, LEN_BUF);
    rawBuf[lenBuf++] = MRK_BEGIN;
    rawBuf[lenBuf++] = NUMB_REC;

    add_record2buff(&rawBuf[lenBuf], &devMsg.record[TIME_REC], &lenBuf);
    add_record2buff(&rawBuf[lenBuf], &devMsg.record[HOST_REC], &lenBuf);
    add_record2buff(&rawBuf[lenBuf], &devMsg.record[SERIAL_REC], &lenBuf);
    add_record2buff(&rawBuf[lenBuf], &devMsg.record[ACTION_REC], &lenBuf);
    add_record2buff(&rawBuf[lenBuf], &devMsg.record[VENDOR_NAME_REC], &lenBuf);
    add_record2buff(&rawBuf[lenBuf], &devMsg.record[VENDOR_ID_REC], &lenBuf);
    add_record2buff(&rawBuf[lenBuf], &devMsg.record[PRODUCT_NAME], &lenBuf);
    add_record2buff(&rawBuf[lenBuf], &devMsg.record[PRODUCT_ID], &lenBuf);


    rawBuf[lenBuf++] = MRK_END;

//    DPRINT("\n**********************************\n");
//    DPRINT("Len_Buf = %d\n", lenBuf);
//    for(i=0; i < lenBuf; i++)
//    {
//        putchar(rawBuf[i]);
//    }
//    DPRINT("\n**********************************\n");

    if (0 != send_buffer_to_server(HostName, PORT, rawBuf, lenBuf) )
    {
        perror("Send UDP message error!");
    }

    if (pFile!=NULL)
    {
      fputs (devMsg.record[TIME_REC].name,pFile);
      fputs(":",pFile);
      fputs (devMsg.record[TIME_REC].value,pFile);
      fputs(" ",pFile);
      fputs (devMsg.record[HOST_REC].name,pFile);
      fputs(":",pFile);
      fputs (devMsg.record[HOST_REC].value,pFile);
      fputs(" ",pFile);
      fputs (devMsg.record[SERIAL_REC].name,pFile);
      fputs(":",pFile);
      fputs (devMsg.record[SERIAL_REC].value,pFile);
      fputs(" ",pFile);
      fputs (devMsg.record[VENDOR_NAME_REC].name,pFile);
      fputs(":",pFile);
      fputs (devMsg.record[VENDOR_NAME_REC].value,pFile);
      fputs(" ",pFile);
      fputs (devMsg.record[VENDOR_ID_REC].name,pFile);
      fputs(":",pFile);
      fputs (devMsg.record[VENDOR_ID_REC].value,pFile);
      fputs(" ",pFile);
      fputs (devMsg.record[PRODUCT_NAME].name,pFile);
      fputs(":",pFile);
      fputs (devMsg.record[PRODUCT_NAME].value,pFile);
      fputs(" ",pFile);
      fputs (devMsg.record[PRODUCT_ID].name,pFile);
      fputs(":",pFile);
      fputs (devMsg.record[PRODUCT_ID].value,pFile);
      fputs(" ",pFile);
      fputs("\n",pFile);
      fclose (pFile);
    }

}

int main (int argc, char *argv[])
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev, *devBlock;

    struct udev_monitor *mon;
    int fd;

    if (argc != 3) {
        fprintf(stderr,"usage: cul [OPTION] [host/file name]\n");
        DPRINT("usage: cul [OPTION] [host/file name]\n");
        exit(1);
    }

    if( 0 == strcmp("-h", argv[1]) )
    {
        strncpy(HostName, /*argv[2]*/ maksimka, LEN_STR);
    }


    /* Create the udev object */
    udev = udev_new();
    if (!udev)
    {
        DPRINT("Can't create udev\n");
        exit(1);
    }


    /* Set up a monitor to monitor disk devices */
    mon = udev_monitor_new_from_netlink(udev, "udev");
    //udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
    udev_monitor_enable_receiving(mon);
    /* Get the file descriptor (fd) for the monitor.
       This fd will get passed to select() */
    fd = udev_monitor_get_fd(mon);


    /* Create a list of the devices in the 'block' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;
        char devnode[LEN_STR];

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


    while (1)
    {
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        /* Check if our file descriptor has received data. */
        if (ret > 0 && FD_ISSET(fd, &fds))
        {
            /* Make the call to receive the device.
               select() ensured that this will not block. */
            dev = udev_monitor_receive_device(mon);
            if (dev)
            {
                write_log(dev);
                udev_device_unref(dev);
            }
        }
        usleep(250*1000);
        DPRINT(".");
        fflush(stdout);
    }

    udev_unref(udev);

    return 0;
}

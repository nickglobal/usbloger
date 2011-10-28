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


int safe_get_udev_str(char* const dest,const char* src)
{
    int status = EINVAL;

    if(NULL != src)
    {
        status = 0;
        strncpy(dest, src, LEN_STR);
    }
    else
    {
        *dest = 0;
    }

    return status;
}

void add_field2buff(char *buff, char *str, int *len)
{
    *buff = strlen(str);
    *len += *buff + 2;
    strcpy(buff+1, str);
    //DPRINT("Field Len: %d\n",*buff);
    //DPRINT("Field Str: %s\n",buff+1);

}

void write_log(struct udev_device *dev)
{
    char curTime[LEN_STR] = {0};
    char curHost[LEN_STR] = {0};
    DevUsb devUsb;
    DevList *devList;
    time_t t;
    int strLen;
    char rawBuf[LEN_BUF];
    int lenBuf;
    const char inserted_str[]={"inserted"};
    FILE * pFile;

    pFile = fopen ("ClientReport.txt","a+");

    t = time(0);
    strncpy(curTime, ctime( &t ), LEN_STR);
    strLen = strlen(curTime);
    curTime[strLen-1] = 0;//flush \n in the time format
    gethostname( curHost, LEN_STR);

    safe_get_udev_str(devUsb.devnode, udev_device_get_devnode(dev));
    safe_get_udev_str(devUsb.serial, udev_device_get_sysattr_value(dev, "serial"));
    safe_get_udev_str(devUsb.vendorId, udev_device_get_sysattr_value(dev, "idVendor"));
    safe_get_udev_str(devUsb.productId, udev_device_get_sysattr_value(dev, "idProduct"));

    if ( EINVAL == safe_get_udev_str(devUsb.action, udev_device_get_action(dev)) )
    {
        safe_get_udev_str(devUsb.action, inserted_str);
    }

    if ( 0 == strcmp(devUsb.action, "add") ||  0 == strcmp(devUsb.action, "inserted") )
    {
        add_dev(&devUsb);
    }
    else if ( 0 == strcmp(devUsb.action, "remove") )
    {
        devList = find_dev(&devUsb);
        if(NULL != devList)
        {
            strcpy(devUsb.serial, devList->serial);
            strcpy(devUsb.vendorId, devList->vendorId);
            strcpy(devUsb.productId, devList->productId);
            remove_dev(&devUsb);
        }

    }

    DPRINT("\n");
    DPRINT("Time: %s\n", curTime);
    DPRINT("Host: %s\n", curHost);
    DPRINT("  Action: %s\n",devUsb.action);
    DPRINT("  Device Node Path: %s\n", devUsb.devnode);
    DPRINT("  serial: %s\n",devUsb.serial);
    DPRINT("  VendorID: %s\n",devUsb.vendorId);
    DPRINT("  ProductID: %s\n",devUsb.productId);
    DPRINT("  devtype : %s\n",udev_device_get_devtype(dev));
    DPRINT("  sysname: %s\n",udev_device_get_sysname(dev));
    DPRINT("  property: %s\n",udev_device_get_property_value(dev, "ID_SERIAL"));

    lenBuf = 0;
    memset(rawBuf, 0,LEN_BUF);
    rawBuf[lenBuf++] = MRK_BEGIN;
    //DPRINT("\n====================\n");
    //DPRINT("Marker Begin: %x\n",rawBuf[lenBuf-1]);
    rawBuf[lenBuf++] = FIELD_NUM;
    //DPRINT("Field Num: %d\n",rawBuf[lenBuf-1]);

    add_field2buff(&rawBuf[lenBuf], curTime, &lenBuf);
    add_field2buff(&rawBuf[lenBuf], curHost, &lenBuf);
    add_field2buff(&rawBuf[lenBuf], devUsb.serial, &lenBuf);
    add_field2buff(&rawBuf[lenBuf], devUsb.vendorId, &lenBuf);
    add_field2buff(&rawBuf[lenBuf], devUsb.productId, &lenBuf);
    add_field2buff(&rawBuf[lenBuf], devUsb.action, &lenBuf);

    rawBuf[lenBuf++] = MRK_END;
    //DPRINT("Marker End: %x\n",rawBuf[lenBuf-1]);
    //DPRINT("\n====================\n");

    if (0 != send_buffer_to_server(IP_ADDRESS, PORT, rawBuf, lenBuf) )
    {
        perror("Send UDP message error!");
    }

    if (pFile!=NULL)
    {
      fputs (curTime,pFile);
      fputs(" ",pFile);
      fputs (curHost,pFile);
      fputs(" ",pFile);
      fputs (devUsb.serial,pFile);
      fputs(" ",pFile);
      fputs (devUsb.vendorId,pFile);
      fputs(" ",pFile);
      fputs (devUsb.productId,pFile);
      fputs(" ",pFile);
      fputs (devUsb.action,pFile);
      fputs("\n",pFile);
      fclose (pFile);
    }

}

int main (void)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev, *devUsb;//, *dev_mmc;

    struct udev_monitor *mon;
    int fd;

//    char time_b[STR_LEN], hostname_b[STR_LEN];
//    time_t t;
//    int str_len;

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

//        dev_mmc = udev_device_get_parent_with_subsystem_devtype(
//               dev,
//               "block",unistd
//               "disk");
//        if (dev_mmc)
//        {

//        }

        devUsb = udev_device_get_parent_with_subsystem_devtype(
               dev,
               "block",
               "disk");
        if (devUsb)
        {
            if ( strcmp(devnode, udev_device_get_devnode(devUsb)) != 0 )
            {
              write_log(devUsb);
              strcpy(devnode, udev_device_get_devnode(devUsb));
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

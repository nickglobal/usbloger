/*
 * ClienUSBLogger.c
 *
 *  Created on: Oct 12, 2011
 *      Author: Igor.Medvyedyev
 */

#include <libudev.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>

#define STR_LEN (255)
#define BEGIN (0xBE)
#define END (0xED)

typedef  struct UsbLog_ {
    char begin;
    char time[STR_LEN];
    char host[STR_LEN];
    char serial[STR_LEN];
    char vendorId[STR_LEN];
    char productId[STR_LEN];
    char action[STR_LEN];
    char end;
}UsbLog;


void write_buff(struct udev_device* dev)
{
    UsbLog log;
    time_t t;
    int str_len;
    char rawBuf[1024];
    FILE * pFile;

    pFile = fopen ("UsbLog.txt","a+");

    log.begin = 0xBE;
    log.end = 0xED;

    t = time(0);
    strcpy(log.time, ctime( &t ));
    str_len = strlen(log.time);
    log.time[str_len-1] = '\0';//flush \n in the time format
    gethostname( log.host, STR_LEN);

    strcpy(log.serial, udev_device_get_sysattr_value(dev, "serial"));
    strcpy(log.vendorId, udev_device_get_sysattr_value(dev, "idVendor"));
    strcpy(log.productId, udev_device_get_sysattr_value(dev, "idProduct"));
    strcpy(log.action, udev_device_get_action(dev));



    if (pFile!=NULL)
    {
      fputs (log.time,pFile);
      fputs(" ",pFile);
      fputs (log.host,pFile);
      fputs(" ",pFile);
      fputs (log.serial,pFile);
      fputs(" ",pFile);
      fputs (log.vendorId,pFile);
      fputs(" ",pFile);
      fputs (log.productId,pFile);
      fputs(" ",pFile);
      fputs (log.action,pFile);
      fputs("\n",pFile);
      fclose (pFile);
    }

//    printf("\nGot Device\n");
//    printf("   Time: %s\n", time_b);
//    printf("   Host: %s\n", hostname_b);
//    printf("   Action: %s\n\n", udev_device_get_action(dev));
//
//    printf("  VendorID: %s\n",
//            udev_device_get_sysattr_value(dev,"idVendor"));
//    printf("  ProductID: %s\n",
//            udev_device_get_sysattr_value(dev, "idProduct"));
//    printf("  serial: %s\n",
//             udev_device_get_sysattr_value(dev, "serial"));
//    printf("  devtype : %s\n",
//            udev_device_get_devtype(dev) );
//    printf("  sysname: %s\n",
//            udev_device_get_sysname(dev) );
}

int main (void)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev, *dev_usb;//, *dev_mmc;

    struct udev_monitor *mon;
    int fd;

//    char time_b[STR_LEN], hostname_b[STR_LEN];
//    time_t t;
//    int str_len;

    /* Create the udev object */
    udev = udev_new();
    if (!udev)
    {
        printf("Can't create udev\n");
        exit(1);
    }


    /* Set up a monitor to monitor disk devices */
    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
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
        char devnode[STR_LEN];

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

//        dev_mmc = udev_device_get_parent_with_subsystem_devtype(
//               dev,
//               "block",unistd
//               "disk");
//        if (dev_mmc)
//        {
//            printf("Device Node Path: %s\n", udev_device_get_devnode(dev_mmc));
//
//            printf("  VendorID: %s\n",
//                    udev_device_get_sysattr_value(dev_mmc,"idVendor"));
//            printf("  ProductID: %s\n",
//                    udev_device_get_sysattr_value(dev_mmc, "idProduct"));
//            printf("  serial: %s\n",
//                     udev_device_get_sysattr_value(dev_mmc, "serial"));
//
//            printf("dev_usb %d\n", dev_mmc);
//        }

        dev_usb = udev_device_get_parent_with_subsystem_devtype(
               dev,
               "usb",
               "usb_device");
        if (dev_usb)
        {
            write_buff(dev_usb);

//            if ( strcmp(devnode, udev_device_get_devnode(dev_usb)) != 0 )
//            {
//            strcpy(devnode, udev_device_get_devnode(dev_usb));
//            printf("  VendorID: %s\n",
//                    udev_device_get_sysattr_value(dev_usb,"idVendor"));
//            printf("  ProductID: %s\n",
//                    udev_device_get_sysattr_value(dev_usb, "idProduct"));
//            printf("  serial: %s\n",
//                     udev_device_get_sysattr_value(dev_usb, "serial"));
//            }
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
                write_buff(dev);
//                t = time(0);
//                strcpy(time_b, ctime( &t ));
//                str_len = strlen(time_b);
//                time_b[str_len-1] = '\0';
//                gethostname( hostname_b, STR_LEN);
//
//                printf("\nGot Device\n");
//                printf("   Time: %s\n", time_b);
//                printf("   Host: %s\n", hostname_b);
//                printf("   Action: %s\n\n", udev_device_get_action(dev));
//
//                printf("  VendorID: %s\n",
//                        udev_device_get_sysattr_value(dev,"idVendor"));
//                printf("  ProductID: %s\n",
//                        udev_device_get_sysattr_value(dev, "idProduct"));
//                printf("  serial: %s\n",
//                         udev_device_get_sysattr_value(dev, "serial"));
//                printf("  devtype : %s\n",
//                        udev_device_get_devtype(dev) );
//                printf("  sysname: %s\n",
//                        udev_device_get_sysname(dev) );



                udev_device_unref(dev);
            }
            else {
                printf("No Device from receive_device(). An error occured.\n");
            }
        }
        usleep(250*1000);
        printf(".");
        fflush(stdout);
    }


    udev_unref(udev);

    return 0;
}

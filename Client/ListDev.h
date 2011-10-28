/*
 * ListDev.h
 *
 *  Created on: Oct 18, 2011
 *      Author: Igor
 */

#ifndef LISTDEV_H_
#define LISTDEV_H_

#include "ClientUSBLogger.h"

typedef struct DevList_s
{
    struct DevList_s *prev;
    struct DevList_s *next;
    char *devnode;
    char *serial;
    char *vendorId;
    char *productId;
}DevList;

void add_dev(DevUsb *dev);
DevList* find_dev(DevUsb *dev);
void remove_dev(DevUsb *dev);


#endif /* LISTDEV_H_ */

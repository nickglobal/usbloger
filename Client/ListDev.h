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
    DevMsg *dev;
}DevList;

void add_dev(DevMsg *dev);
DevList* find_dev(DevMsg *dev);
void remove_dev(DevMsg *dev);


#endif /* LISTDEV_H_ */

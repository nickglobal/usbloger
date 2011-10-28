

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ListDev.h"
#include "ListDev.h"

#define ALLOCATE_STR(dest, str) \
        dest = malloc(strlen(str) + 1); \
        strcpy(dest, str);

#define SAFE_DEREFER_ASSIGN(pointer, field, value) \
        if (NULL != pointer) \
        { \
            pointer->field = value; \
        } \

DevList *ListEnd;

void alloc_dev_str(DevList *list, DevUsb *dev)
{
    ALLOCATE_STR(list->devnode, dev->devnode);
    ALLOCATE_STR(list->serial, dev->serial);
    ALLOCATE_STR(list->vendorId, dev->vendorId);
    ALLOCATE_STR(list->productId, dev->productId);
}

void add_dev(DevUsb *dev)
{
    if( NULL == ListEnd)
    {//First list node
        ListEnd = malloc(sizeof(DevList));
        if(NULL == ListEnd)
        {
            perror("Error: NULL was returned by malloc()");
        }
        else
        {
            ListEnd->next = NULL;
            ListEnd->prev = NULL;
            alloc_dev_str(ListEnd, dev);
        }
   }
    else
    {
        ListEnd->next = malloc(sizeof(DevList));
        if(NULL == ListEnd->next)
        {
            perror("Error: NULL was returned by malloc()");
        }
        else
        {
            ListEnd->next->next = NULL;
            ListEnd->next->prev = ListEnd;
            ListEnd = ListEnd->next;
            alloc_dev_str(ListEnd, dev);
        }
    }//else if( NULL == ListEnd)
}

void remove_dev(DevUsb *dev)
{
    DevList *list;

    list = find_dev(dev);

    if (NULL != list)
    {
        SAFE_DEREFER_ASSIGN(list->prev, next, list->next);
        SAFE_DEREFER_ASSIGN(list->next, prev, list->prev);
        if (ListEnd == list)
        {
            ListEnd = list->prev;
        }
        free(list->serial);
        free(list->vendorId);
        free(list->productId);
        free(list);
    }
}

DevList* find_dev(DevUsb *dev)
{
    DevList *list, *matchItem;

    matchItem = NULL;
    list = ListEnd;

    while(NULL != list)
    {//Iterate list from end to begin
        if(0 != strcmp(dev->devnode,  list->devnode) )
        {
            list = list->prev;
        }
        else
        {
            matchItem = list;
            break;
        }
    }

    return matchItem;
}


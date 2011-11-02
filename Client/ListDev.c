

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ListDev.h"
#include "ListDev.h"

#define SAFE_DEREFER_ASSIGN(pointer, field, value) \
        if (NULL != pointer) \
        { \
            pointer->field = value; \
        } \

DevList *ListEnd;

void allocate_dev(DevList *list, DevMsg *dev)
{
    list->dev = malloc(sizeof(DevMsg));
    memcpy(list->dev, dev, sizeof(DevMsg));
}

void add_dev(DevMsg *dev)
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
            allocate_dev(ListEnd, dev);
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
            allocate_dev(ListEnd, dev);
        }
    }//else if( NULL == ListEnd)
}

void remove_dev(DevMsg *dev)
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
        free(list->dev);
        free(list);
    }
}

DevList* find_dev(DevMsg *dev)
{
    DevList *list, *matchItem;

    matchItem = NULL;
    list = ListEnd;

    while(NULL != list)
    {//Iterate list from end to begin
        if(0 != strcmp(dev->devnode,  list->dev->devnode) )
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


/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 03/04/2014
* @description socket utilities header
*/

#ifndef TRANSPORTUTIL_H
#define TRANSPORTUTIL_H

#include <string>

namespace transport {


#define MAX_EPOLL_EVENTS 20
#define RECV_SIZE 1024*4
#define MAX_RECV_SIZE 1024*1024
#define DEFAULT_BUFF_CAPACITY 1024*10

struct FdInfo
{
    char * uri; // apply to ethier socket (url) or file (path)
    char * buff; // size 8
    size_t buff_offset;
    size_t buff_capacity; // size 8
    int    fd; // size 4
};

int getEpollfd ( );

void freeFdInfo (struct FdInfo * );

int connectTo (const char * ip, int port );

int send (struct FdInfo * );

int recv (struct FdInfo * );

int setNonblock (int sockfd );

}; // end of package namespace

#endif

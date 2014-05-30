/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 03/04/2014
* @description socket utilities
*/

#include "transportUtil.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#include "lib/Logger.h"

namespace transport {


int getEpollfd ( )
{
    static int epollfd = 0;

    if (0 == epollfd) {
        if (-1 == (epollfd = epoll_create(MAX_EPOLL_EVENTS))) {
            LOG(ERROR) << "epoll_create() failed. errno " << errno;
            exit(EXIT_FAILURE);
        }
        LOG(DEBUG3) << "epoll_create() succeed. fd = " << epollfd;
    }

    return epollfd;
}


void freeFdInfo (struct FdInfo * fdinfo )
{
    if (nullptr == fdinfo)
        return;

    free(fdinfo->uri);
    free(fdinfo->buff);
    free(fdinfo);
    fdinfo = nullptr;
}


int connectTo (const char * host, int port )
{
    static bool set_hints = true;
    static struct addrinfo hints;
    if (set_hints) {
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;
        set_hints = false;
    }

    char portstr[5];
    sprintf(portstr, "%d", port);
    struct addrinfo *addr, *addrptr;
    if (0 != getaddrinfo(host, portstr, &hints, &addr)) {
        LOG(WARNING) << "getaddrinfo() on " << host << " failed";
        return -1;
    }

	char ipstr[INET_ADDRSTRLEN];
	int sockfd;
    for (addrptr = addr; addrptr != nullptr; addrptr = addrptr->ai_next) {
        sockfd = socket(addrptr->ai_family, addrptr->ai_socktype, 
                        addrptr->ai_protocol);
        if (-1 == sockfd) {
            continue;
        }

        if (-1 != connect(sockfd, addrptr->ai_addr, addrptr->ai_addrlen)) {
            inet_ntop(AF_INET, 
                      &(((struct sockaddr_in *)(addrptr->ai_addr))->sin_addr),
                      ipstr, 16);
            LOG(DEBUG3) << "Connect to " << host << " succeed. sockfd = " << sockfd;
            break;
        }

        close(sockfd);
    }

    if (nullptr == addrptr) {
        LOG(WARNING) << "Could not connect to " << host;
    }

    freeaddrinfo(addr);
    setNonblock(sockfd);

	return sockfd;	
}


int send (struct FdInfo * fdinfo )
{
    if ((nullptr == fdinfo) || (nullptr == fdinfo->buff)) {
        return -1;
    }

    int n = 0;
    int sockfd = fdinfo->fd;
    char * writebuff = fdinfo->buff;
    int offset = fdinfo->buff_offset;
    int rest = strlen(fdinfo->buff) - offset;

    // TODO restrict write count to a constant rather than rest
    while (rest > 0) {
        n = write(sockfd, writebuff + offset, rest);
        LOG(DEBUG1) << "write to fd " << sockfd << " " << n << " bytes";
        if (n >= 0) {
            offset += n;
            rest -= n;
        }
        else if (EAGAIN == errno) {
            fdinfo->buff_offset = offset;
            return -1;
        }
        else {
            LOG(WARNING) << "write to fd " << sockfd << " failed. errno " << errno;
            return -1;
        }
    }

	return 0;
}


int recv (struct FdInfo * fdinfo )
{
    if (nullptr == fdinfo) {
        return -1;
    }

    if (nullptr == fdinfo->buff) {
        fdinfo->buff = (char *)calloc(DEFAULT_BUFF_CAPACITY, sizeof(char));
        fdinfo->buff_capacity = DEFAULT_BUFF_CAPACITY;
        fdinfo->buff_offset = 0;
    }

    int n = -1;
    int offset = fdinfo->buff_offset;

    while ((n = read(fdinfo->fd, fdinfo->buff + offset, RECV_SIZE)) != 0) {
        if (n > 0) {
            offset += n;
            if (offset + RECV_SIZE >= MAX_RECV_SIZE) {
                LOG(WARNING) << "read from fd " << fdinfo->fd << 
                    "failed due to recv exceeds max allowed size" << MAX_RECV_SIZE;
                return -1;
            }
            else if (offset + RECV_SIZE >= fdinfo->buff_capacity) {
                fdinfo->buff_capacity *= 2;
                char * tmp = (char *)realloc(fdinfo->buff, fdinfo->buff_capacity);
                if (nullptr == tmp) {
                    LOG(WARNING) << "read from fd " << fdinfo->fd << " failed due to realloc failed";
                    return -1;
                }
                fdinfo->buff = tmp;
            }
        }
        else if (EAGAIN == errno) {
            // read may not complete. put offset back to fdinfo for next read
            LOG(DEBUG1) << "read from fd " << fdinfo->fd << " recved EAGAIN";
            fdinfo->buff_offset = offset;
            return -1;
        }
        else {
            LOG(WARNING) << "read from fd " << fdinfo->fd << " failed. errno " << errno;
            return -1;
        }
    }

    // if close fd at this point, the released fd maybe reused and then 
    //  epoll_ctl(EPOLL_CTL_DEL, fd) will have problem
    fdinfo->buff[offset] = '\0';
	return offset;
}


int setNonblock (int sockfd )
{
	int flag;
	if (-1 == (flag = fcntl(sockfd, F_GETFL, 0)))
		return -1;

	flag |= O_NONBLOCK;
	if (-1 == fcntl(sockfd, F_SETFL, flag))
		return -1;
	return 0;
}

};

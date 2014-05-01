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
            //fprintf(stderr, "epoll_create() failed. errno %d\n", errno);
            exit(EXIT_FAILURE);
        }
        LOG(INFO) << "epoll_create() succeed. fd = " << epollfd;
        //fprintf(stderr, "epoll fd = %d\n", epollfd); // debug
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


int connectTo (const char * ip, int port )
{
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(port);
	memcpy(&servaddr.sin_addr, ip, 4);

	int sockfd;
	if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
        LOG(ERROR) << "socket error. errno " << errno;
		//fprintf(stderr, "    HTTP. socket error, errno: %d\n", errno);
		return -1;
	}

	if (-1 == connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        LOG(ERROR) << "connect error. errno " << errno;
		//fprintf(stderr, "    HTTP. connect error, errno: %d\n", errno);
		return -1;
	}

	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, ip, ipstr, sizeof(ipstr));
    LOG(DEBUG3) << "connection to " << ipstr << " succeed. sockfd " << sockfd;
	//fprintf(stderr, "    HTTP. Connection to %s:%d succeed. sockfd %d\n",
	//	    ipstr, ntohs(servaddr.sin_port), sockfd);
    
    //setNonblock(sockfd);

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
        //fprintf(stderr, "    HTTP. write to fd %d %d bytes.\n", sockfd, n);
        if (n >= 0) {
            offset += n;
            rest -= n;
        }
        else if (EAGAIN == errno) {
            fdinfo->buff_offset = offset;
            return -1;
        }
        else {
            LOG(ERROR) << "write to fd " << sockfd << " failed. errno " << errno;
            //fprintf(stderr, "    HTTP. write to fd %d failed. errno %d\n", 
            //        sockfd, errno);
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
        fdinfo->buff = (char *)malloc(DEFAULT_BUFF_CAPACITY);
        fdinfo->buff_capacity = DEFAULT_BUFF_CAPACITY;
        fdinfo->buff_offset = 0;
    }

    LOG(DEBUG1) << "read fd " << fdinfo->fd;
    //fprintf(stderr, "    HTTP. read fd %d\n", fdinfo->fd);
    int n = -1;
    int offset = fdinfo->buff_offset;

    while ((n = read(fdinfo->fd, fdinfo->buff + offset, RECV_SIZE)) != 0) {
        if (n > 0) {
            offset += n;
            if (offset + RECV_SIZE >= MAX_RECV_SIZE) {
                LOG(DEBUG1) << "recv exceeds max allowed size" << MAX_RECV_SIZE;
                //fprintf(stderr, "    HTTP. recv exceeds max length %d\n", MAX_RECV_SIZE);
                return -1;
            }
            else if (offset + RECV_SIZE >= fdinfo->buff_capacity) {
                fdinfo->buff_capacity *= 2;
                char * tmp = (char *)realloc(fdinfo->buff, fdinfo->buff_capacity);
                if (nullptr == tmp) {
                    return -1;
                }
                fdinfo->buff = tmp;
            }
        }
        else if (EAGAIN == errno) {
            // read may not complete. put offset back to fdinfo for next read
            LOG(DEBUG1) << "read from fd " << fdinfo->fd << " recved EAGAIN";
            //fprintf(stderr, "    HTTP. read from fd %d recved EAGAIN\n", fdinfo->fd);
            fdinfo->buff_offset = offset;
            return -1;
        }
        else {
            LOG(DEBUG1) << "read from fd " << fdinfo->fd << " failed. errno " << errno;
            //fprintf(stderr, "    HTTP. read from fd %d failed, errno %d\n", 
            //        fdinfo->fd, errno);
            return -1;
        }
    }

    // if close fd at this point, the released fd maybe reused and then 
    //  in epoll_ctl(EPOLL_CTL_DEL, fd) has issue
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
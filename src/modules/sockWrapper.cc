/**
* Coopyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description socket client functions
*/

#include "sockWrapper.h"

#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

namespace sock {


int connectTo (char * ip, int port )
{
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(port);
	memcpy(&servaddr.sin_addr, ip, 4);

	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("socket error, errno: %d\n", errno);
		return -1;
	}

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		printf("connect error, errno: %d\n", errno);
		return -1;
	}

	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, ip, ipstr, sizeof(ipstr));
	//printf("Connection to %s:%d succeed.\n",
	//	ipstr, ntohs(servaddr.sin_port));
	return sockfd;	
}


int sendRequest (int sockfd, const char * req_msg )
{
	int n, done, rest;
	done = 0;
	rest = strlen(req_msg);
	while (rest > 0) {
		n = write(sockfd, req_msg + done, rest);
		if (n <= 0) {
			if (errno == EAGAIN) {
				usleep(200000);
				continue;
			}
			printf("Sending request error\n");
			close(sockfd);
			return -1;
		}
		done += n;
		rest -= n;
	}
	return 0;
}


int recvResponse (int sockfd, char * recv_msg )
{
	int n, recv_len, header_len;
	n = -1;
	recv_len = header_len = 0;
	
	while (n != 0) {
		n = read(sockfd, recv_msg + recv_len, 4096);
		if (n > 0) {
			recv_len += n;
		}
		else if (n < 0) {
/*			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				printf("recv_response. errno = EAGAIN#\n");
				usleep(200000);
				continue;        // For non_block
			}
			else*/ if (errno == EINTR) {
				printf("recv interruputed#\n");
				close(sockfd);
				return -1;
			}
		}
		//printf("recv_response. n = %d, recv_len = %d#\n", n, recv_len);
	}
	close(sockfd);

	return recv_len;
}


int setNonblock (int sockfd )
{
	int flag;
	if ((flag = fcntl(sockfd, F_GETFL, 0)) == -1)
		return -1;

	flag |= O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, flag) == -1)
		return -1;
	return 0;
}

};

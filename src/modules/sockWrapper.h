/**
* Coopyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description socket client functions header
*/

#ifndef SOCKWRAPPER_H
#define SOCKWRAPPER_H

#include <string>

#define MAX_REQ_LEN 1024
#define MAX_RESP_LEN 1024*1024

namespace sock {


int connectTo (char * ip, int port );

int sendRequest (int sockfd, const char * req_msg );

int recvResponse (int sockfd, char * recv_msg );

int setNonblock (int sockfd );

}; // end of package namespace

#endif // SOCKWRAPPER_H

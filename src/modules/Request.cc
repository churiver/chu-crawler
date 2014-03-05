/**
* Coopyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description HTTP Request
*/

#include "Request.h"

#include <cstdio>
#include <cstring>

#include <string>

#include "sockWrapper.h"
#include "Response.h"

namespace http {


const std::string Request::METHOD_GET = "GET";
const std::string Request::HEADER_HOST = "HOST";
const std::string Request::HEADER_AGENT = "User-Agent";
const std::string Request::HEADER_CONN = "Connection";
const std::string Request::HEADER_ACCP = "Accept";


Request::Request (const std::string & url, const std::string & req_method )
{    
    raw_url = url;
    parsed_url = new ParsedUrl(url.c_str());
    method = req_method;
    header_map[HEADER_HOST] = parsed_url->host;
}


Request::~Request ()
{
    delete   parsed_url;
    delete   resp;
}


void Request::setReqMessage(char * req_msg)
{
    sprintf(req_msg, 
            "%s %s HTTP/1.1\r\n", 
            method.c_str(), parsed_url->path);

    std::map<std::string, std::string>::iterator it;
    for (it = header_map.begin(); it != header_map.end(); it++)
        sprintf(req_msg + strlen(req_msg), 
                "%s: %s\r\n",
                it->first.c_str(), it->second.c_str());
    snprintf(req_msg + strlen(req_msg), 
             3, 
             "%s", 
             "\r\n");
    
    //printf("req_msg: \n%s\n", req_msg);
}


const State Request::execute (const bool is_nonblock )
{
    if (parsed_url->state != OK) {
        printf("parse url failed. err code %d\n", parsed_url->state);
        return parsed_url->state;
    }

    int sockfd;
    if ((sockfd = sock::connectTo(parsed_url->ip, parsed_url->port)) < 0)
        return ERR_REQ_CONNECT;

    if (is_nonblock)
        sock::setNonblock(sockfd);

    char req_msg[MAX_REQ_LEN] = {0};
    setReqMessage(req_msg);
    sock::sendRequest(sockfd, req_msg);

    char recv_msg[MAX_RESP_LEN] = {0};
    if (sock::recvResponse(sockfd, recv_msg) == 0)
        return ERR_REQ_RESPONSE;

    resp = new Response(recv_msg);

    return OK;
}

};

/*int main ( )
{
    std::string url("http://www.google.com");
    http::Request req(url, http::Request::METHOD_GET);
    req.setHeader(http::Request::HEADER_CONN, "close");
    req.execute();
    return 0;
}*/

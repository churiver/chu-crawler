/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 03/04/2014
* @description http utilities header
*/

#ifndef HTTPUTIL_H
#define HTTPUTIL_H

#include <string>

namespace http {


extern const std::string CONTENT_TYPE;
extern const std::string CONTENT_ENCODING;
extern const std::string CONTENT_TYPE_HTML;

enum State
{
    OK, ERR_URL_EMPTY, ERR_URL_SCHEME, ERR_URL_PORT, ERR_URL_IP,
    ERR_REQ_CONNECT, ERR_REQ_RESPONSE,
    ERR_RESP_INVALID        
};

char * setRequest (const char *, const char * );

int validateUrl (const std::string & );

int normalizeUrl (std::string & );

int resolveUrl (std::string &, const std::string &, const std::string & );

}; // end of package namespace

#endif // UTIL_H

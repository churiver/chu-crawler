/**
* Coopyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description HTTP utilities header
*/

#ifndef UTIL_H
#define UTIL_H

#include <string>

namespace http {


enum State
{
    OK, ERR_URL_EMPTY, ERR_URL_SCHEME, ERR_URL_PORT, ERR_URL_IP,
    ERR_REQ_CONNECT, ERR_REQ_RESPONSE,
    ERR_RESP_INVALID        
};

int checkUrlType (const std::string & );

int normalizeUrl (std::string & );

int resolveUrl (std::string &, const std::string &, const std::string & );

}; // end of package namespace

#endif // UTIL_H

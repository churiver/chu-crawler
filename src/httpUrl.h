/**
* Coopyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create date 03/04/2014
* @description Parsed URL header
*/

#ifndef HTTPURL_H
#define HTTPURL_H

#include <string>

#include "httpUtil.h"

namespace http {


class Url
{
public:
    Url (const char *, bool host_path_only = false);

    Url (const Url & );

    Url & operator= (const Url & );

    ~Url ( );

    const std::string & getStr ( ) const;

    State        getState ( ) const;

    int          getPort ( ) const;

    const char * getIp ( ) const;

    const char * getHost ( ) const;

    const char * getPath ( ) const;

private:
    std::string urlstr;
    State  state;
    int    port;
    char * ip;
    char * scheme;
    char * host;
    char * path;
};
}; // end of package namespace

#endif

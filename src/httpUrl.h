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
    Url (const char *);

    Url (const Url & );

    Url & operator= (const Url & );

    ~Url ( );

    const std::string & getStr ( ) const;

    State getState ( ) const;

    int getPort ( ) const;

    const char * getScheme ( ) const;

    const char * getHost ( ) const;

    const char * getPath ( ) const;

    void output ( );

private:
    std::string _urlstr;
    State  _state;
    int    _port;
    char * _scheme;
    char * _host;
    char * _path;
};

};

#endif

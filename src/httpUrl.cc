/**
* Coopyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create date 03/04/2014
* @description Parsed URL
*/

#include "httpUrl.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <netdb.h>
#include <arpa/inet.h>

namespace http {


Url::Url (const char * url)
    : _port(80), _state(OK), _scheme(nullptr),
        _host(nullptr), _path(nullptr)
{
    if (url == nullptr) {
        _state = ERR_URL_EMPTY;
        return;
    }
    const char * url_ptr = url;
    _urlstr = std::string(url_ptr);
    
    const char * scheme_end = strstr(url_ptr, "://");
    if (scheme_end != nullptr) {
        scheme_end += 3; // skip "://"
        _scheme = strndup(url_ptr, scheme_end - url_ptr - 3);
    }
    else {
        scheme_end = url_ptr;
        _scheme = strdup("http");
    }
    
    const char * port_start = strchr(scheme_end, ':');
    const char * path_start = strchr(scheme_end, '/');
    if ((port_start != nullptr)) {
        if ((path_start == nullptr) || (port_start < path_start)) {
            if (0 == (_port = atoi(port_start + 1))) {
                _state = ERR_URL_PORT;
                return;
            }
        }
    }
    else if (0 == strcmp("https", _scheme)) {
        _port = 443;
    }

    if (path_start == nullptr) {
        _host = strdup(scheme_end);
        _path = strdup("/");
    }
    else {
        _host = strndup(scheme_end, path_start - scheme_end);
        _path = strdup(path_start);
    }
/*
    if (true == no_ip) {
        return;
    }


    struct hostent * hptr;
    if (((hptr = gethostbyname(_host)) == nullptr) ||
         (hptr->h_addr_list[0] == nullptr)) {
        _state = ERR_URL_IP;
        return;
    }
    _ip = strdup(hptr->h_addr_list[0]); */
}


Url::Url (const Url & rhs )
    : _port(rhs._port) 
{
    _scheme = strdup(rhs._scheme);
    _host = strdup(rhs._host);
    _path = strdup(rhs._path);
}


Url & Url::operator= (const Url & rhs )
{
    if (this == &rhs)
        return *this;
    _scheme = strdup(rhs._scheme);
    _host = strdup(rhs._host);
    _port = rhs._port;
    _path = strdup(rhs._path);
    return *this;
}


Url::~Url ( )
{
    delete[] _scheme;
    delete[] _host;
    delete[] _path;
}


State Url::getState ( ) const
{
    return _state;
}


int Url::getPort ( ) const
{
    return _port;
}


const std::string & Url::getStr ( ) const
{
    return _urlstr;
}


const char * Url::getScheme ( ) const
{
    return _scheme;
}


const char * Url::getHost ( ) const
{
    return _host;
}


const char * Url::getPath ( ) const
{
    return _path;
}


void Url::output ( )
{
    if (_state != OK)
        fprintf(stderr, "Url invalid. err_state %d\n", _state);
    else
        fprintf(stderr, "\tUrl OK.\nscheme: %s\nhost: %s\n"
                "port: %d\npath: %s\n\n",
                _scheme, _host, _port, _path);
}

};

/*
int main() {
    std::string s("http://en.wikipedia.org/wiki/Template:Disclaimers");
    http::Url url(s.c_str());
    url.output();
}
*/

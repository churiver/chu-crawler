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


Url::Url (const char * url, bool host_path_only )
    : port(80), state(OK), scheme(nullptr),
        host(nullptr), path(nullptr), ip(nullptr)
{
    if (url == nullptr) {
        state = ERR_URL_EMPTY;
        return;
    }
    
    urlstr = std::string(url);
    const char * url_ptr = url;
    
    const char * scheme_end = strstr(url_ptr, "://");
    if (scheme_end != nullptr) {
        scheme_end += 3; // skip "://"
        scheme = strndup(url_ptr, scheme_end - url_ptr - 3);
    }
    else {
        scheme_end = url_ptr;
    }
    
    const char * port_start = strchr(scheme_end, ':');
    if ((port_start != nullptr) && 
            (0 == (port = atoi(port_start + 1)))) {
        state = ERR_URL_PORT;
        return;
    }

    const char * path_start = strchr(scheme_end, '/');
    if (path_start == nullptr) {
        host = strdup(scheme_end);
        path = strdup("/");
    }
    else {
        host = strndup(scheme_end, path_start - scheme_end);
        path = strdup(path_start);
    }

    if (true == host_path_only) {
        return;
    }

    struct hostent * hptr;
    if (((hptr = gethostbyname(host)) == nullptr) ||
         (hptr->h_addr_list[0] == nullptr)) {
        state = ERR_URL_IP;
        return;
    }
    ip = strdup(hptr->h_addr_list[0]); 
}


Url::Url (const Url & rhs )
    : port(rhs.port) 
{
    scheme = strdup(rhs.scheme);
    host = strdup(rhs.host);
    path = strdup(rhs.path);
    ip = strdup(rhs.ip);
}


Url & Url::operator= (const Url & rhs )
{
    if (this == &rhs)
        return *this;
    scheme = strdup(rhs.scheme);
    host = strdup(rhs.host);
    port = rhs.port;
    path = strdup(rhs.path);
    ip = strdup(rhs.ip);
    return *this;
}


Url::~Url ( )
{
    delete[] scheme;
    delete[] host;
    delete[] path;
    delete[] ip;
}


State        Url::getState ( ) const
{
    return state;
}


int          Url::getPort ( ) const
{
    return port;
}


const std::string & Url::getStr ( ) const
{
    return urlstr;
}


const char * Url::getIp ( ) const
{
    return ip;
}


const char * Url::getHost ( ) const
{
    return host;
}


const char * Url::getPath ( ) const
{
    return path;
}

};

/*
void Url::output ( )
{
    if (state != OK)
        fprintf(stderr, "Url invalid. err_state %d\n", state);
    else
        fprintf(stderr, "\tUrl OK.\nscheme: %s\nhost: %s\n"
                "port: %d\npath: %s\nip: %x\n\n",
                scheme, host, port, path, ip);
}
*/

/*int main() {
    std::string s("http://www.baidu.com");
    http::Url url(s);
    url.output();
}
*/

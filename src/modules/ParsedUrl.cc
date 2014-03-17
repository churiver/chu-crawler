/**
* Coopyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description Parsed URL
*/

#include "ParsedUrl.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>

#include "util.h"

namespace http {


ParsedUrl::ParsedUrl (const char * url)
    : port(80), state(OK), scheme(nullptr), 
        host(nullptr), path(nullptr), ip(nullptr)
{
    if (url == nullptr) {
        state = ERR_URL_EMPTY;
        return;
    }

    const char * url_ptr = url;
    
    const char * scheme_end = strstr(url_ptr, "://");
    if (scheme_end == nullptr) {
        state = ERR_URL_SCHEME;
        return;
    }
    scheme_end += 3; // skip "://"
    scheme = strndup(url_ptr, scheme_end - url_ptr - 3);
    
    const char * port_start = strchr(scheme_end, ':');
    if (port_start != nullptr)
        port = atoi(port_start + 1);
    if (port == 0) {
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

    struct hostent * hptr;
    if (((hptr = gethostbyname(host)) == nullptr) ||
         (hptr->h_addr_list[0] == nullptr)) {
        state = ERR_URL_IP;
        return;
    }
    ip = strdup(hptr->h_addr_list[0]); 
}


ParsedUrl::ParsedUrl (const ParsedUrl & rhs )
    : port(rhs.port) 
{
    scheme = strdup(rhs.scheme);
    host = strdup(rhs.host);
    path = strdup(rhs.path);
    ip = strdup(rhs.ip);
}


ParsedUrl & ParsedUrl::operator= (const ParsedUrl & rhs )
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


ParsedUrl::~ParsedUrl ( )
{
    delete[] scheme;
    delete[] host;
    delete[] path;
    delete[] ip;
}


void ParsedUrl::output ( )
{
    if (state != OK)
        fprintf(stderr, "ParsedUrl invalid. err_state %d\n", state);
    else
        fprintf(stderr, "\tParsedUrl OK.\nscheme: %s\nhost: %s\n"
                "port: %d\npath: %s\nip: %x\n\n",
                scheme, host, port, path, ip);
}

};


/*int main() {
    std::string s("http://www.baidu.com");
    http::ParsedUrl url(s);
    url.output();
}
*/

/**
* Coopyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description Parsed URL header
*/

#ifndef PARSEDURL_H
#define PARSEDURL_H

#include <string>

#include "util.h"

namespace http {


class ParsedUrl
{
    friend class Request;
    friend class Response;


private:

    char * scheme;
    char * host;
    int port;
    char * path;
    char * ip;
    State state;

    ParsedUrl (const char * );

    ParsedUrl (const ParsedUrl & );

    ParsedUrl & operator= (const ParsedUrl &);

    ~ParsedUrl ( );

    void output ( );

};
}; // end of package namespace

#endif // PARSEDURL_H

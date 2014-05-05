/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 03/04/2014
* @description http response header
*/

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <map>

#include "httpUtil.h"

namespace http {


class Response
{
public:

    Response (const char * );

    ~Response ( );

    int getStatusCode ( ) const;

    const std::string & getHeader (const std::string );

    const std::string & getBody ( ) const;

    void output (bool print_body = false );

    //int download (const std::string &, const std::string & );
    
    //int download (const char *, const char * );

private:

    State _state;
    int _status_code;
    std::string _body;
    std::map<std::string, std::string> _header_map;

    int setStatusCode (const char *, int ); 

    void setHeaders (const char *, int );

};

};

#endif

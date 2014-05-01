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


    const std::string & getHeader (const std::string key)
    {
        return header_map[key]; // key can't be reference
    }


    int getStatusCode ( ) const
    {
        return status_code;
    }


    const std::string & getBody ( ) const
    {
        return body;
    }


    void output (bool print_body = false );

    int download (const std::string &, const std::string & );
    
    int download (const char *, const char * );


private:

    State state;
    int status_code;
    std::string body;
    std::map<std::string, std::string> header_map;

    int setStatusCode (const char *, int ); 

    void setHeaders (const char *, int );

};

};

#endif

/**
* Coopyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description HTTP Response header
*/

#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <map>

#include "util.h"

namespace http {


class Response
{
public:

	const static std::string CONTENT_TYPE;
	const static std::string CONTENT_ENCODING;
	const static std::string CONTENT_LENGTH;

	const static std::string CONTENT_TYPE_HTML;
	const static std::string CONTENT_TYPE_PIC;

    Response (char * );

    ~Response ( );


    const std::string & getHeader (const std::string key)
    {
        return header_map[key]; // key can't be reference
    }


    const int getStatusCode ( ) const
    {
        return status_code;
    }


    const std::string & getBody ( ) const
    {
        return body;
    }


    void output (bool print_body = false );

    int downloadFile (const std::string &, const std::string & );


private:

    int status_code;
    std::map<std::string, std::string> header_map;
    std::string body;
    
    State state;

    int setStatusCode (char *, int ); 
    void setHeaders (char *, int );

    const char * getNameByUrl (const std::string & );
    const char * getNameByUrl (const char * url );

};
}; // end of package namespace

#endif // RESPONSE_H

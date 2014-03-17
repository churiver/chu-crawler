/**
* Coopyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @date 03/04/2014
* @description HTTP Request header
*/

#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>

#include "ParsedUrl.h"

namespace http {


class Response;

class Request
{
public:

    const static std::string METHOD_GET;
    const static std::string HEADER_HOST;
    const static std::string HEADER_AGENT;
    const static std::string HEADER_CONN;
    const static std::string HEADER_ACCP;


    Request (const std::string &, const std::string & = METHOD_GET);

    ~Request ( );


    void setHeader (const std::string key, const std::string value )
    {
        header_map[key] = value;
    }


    const std::string & getHeader (const std::string key )
    {
        return header_map[key];
    }


    const std::string getBaseHost ( )
    {
        std::size_t scheme_end = raw_url.find("://");
        scheme_end = (scheme_end == std::string::npos) ? 0 : scheme_end + 3;
        std::size_t host_end = raw_url.find('/', scheme_end);
        if (host_end == std::string::npos) 
            return raw_url;
        return raw_url.substr(0, host_end);
        // return host without trailing /
    }


    const std::string getPath ( )
    {
        std::string path = std::string(parsed_url->path);
        std::size_t path_end = path.rfind('/');
        if (path_end == std::string::npos)
            return "/"; // if there's no "/" then path is root
        return path.substr(0, path_end + 1);
    }


    Response & getResponse ( )
    {
        return *resp;
    }


    /**
     * send request to host and receive response, parse response to header and content
     */
    const State execute (const bool is_nonblock = false );


private:

    std::string raw_url;
    ParsedUrl * parsed_url;
    std::string method;
    std::map<std::string, std::string> header_map;
    Response * resp;

    void setReqMessage(char *);

};
}; // end of package namespace

#endif // REQUEST_H

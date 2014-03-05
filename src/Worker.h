/**
* Copyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver at gmail.com
* @date 03/04/2014
* @description Crawler header
*/

#ifndef WORKER_H
#define WORKER_H

#include <string>
#include <set>
#include <list>
#include <queue>

#include "modules/Request.h"
#include "modules/Response.h"

class Worker
{
public:

    Worker (int id );

    ~Worker ( );

    int getId ( ) {return id;}

    void start (const char * );

    /**
     * crawlUrl:
     * 1. create httpRequest object from url.
     * 2. set request object with headers
     * 3. get httpResponse via request.execute
     * 4. clear pendingLink at last
     */
    int crawlUrl (std::string & url );


private:

    static std::set<std::string> known_url_set;
    static std::set<std::string> bad_url_set;

    unsigned int id;
    std::queue<std::string> pending_url_que;


    /**
     * get all links
     */
    int reapLinks (http::Response &, std::list<std::string> & );

    /**
     * iterate links, filter out non-qualified ones, resolve relative ones
     */
    int processLinks (http::Request &, std::list<std::string> & );

    int checkResponse (http::Response & );

};

#endif // WORKER_H

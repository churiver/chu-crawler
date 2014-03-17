/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver at gmail.com
* @date 03/04/2014
* @description Crawler header
*/

#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>

#include <string>
#include <set>
#include <list>
#include <queue>

#include "modules/Request.h"
#include "modules/Response.h"

class Worker
{
public:

    Worker ( );

    ~Worker ( );

    int getId ( ) {return id;}

    void start (const char * );

    void start (const std::string );

    /**
     * crawlUrl:
     * 1. create httpRequest object from url.
     * 2. set request object with headers
     * 3. get httpResponse via request.execute
     * 4. clear pendingLink at last
     */
    int crawlUrl (std::string & url );

    static unsigned int max_url_count;
    static std::string download_path;

private:

    static std::set<std::string> known_url_set;
    static std::set<std::string> bad_url_set;
    static pthread_mutex_t known_url_mutex;
    static pthread_mutex_t bad_url_mutex;
    static pthread_mutex_t download_url_mutex;
    static unsigned int instance_count;
    static unsigned int next_id;
    static unsigned int download_url_count; // total urls downloaded among threads

    unsigned int id;
    unsigned int url_count; // urls downloaded in current thread
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

    bool isTargetAchieved ( );

};

#endif // WORKER_H

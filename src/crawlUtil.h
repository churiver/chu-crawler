/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/19/2014
* @description crawl utilities header
*/

#ifndef CRAWLUTIL_H
#define CRAWLUTIL_H

#include <pthread.h>

#include <set>
#include <string>
#include <list>

#include "httpUrl.h"
#include "httpResponse.h"

namespace crawl {


// replace with struct conf when params increase

void init ( );

void destroy ( );

int processResponse (http::Response &, http::Url & );

int reapLink (http::Response &, std::list<std::string> & );

int processLink (http::Url &, std::list<std::string> & );

const std::string urlToFileName (std::string );

const char * urlToFileName (const char * );

void downloadText (const std::string &, const std::string & );

};

#endif

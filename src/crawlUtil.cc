/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/19/2014
* @description crawl utilities
*/

#include "crawlUtil.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>

#include <fstream>

#include "lib/ThreadPool.h"
#include "lib/Logger.h"
#include "httpUtil.h"
#include "transportUtil.h"

extern std::string g_download_dir;

namespace crawl {


std::set<std::string> valid_urlset;
std::set<std::string> invalid_urlset;
pthread_mutex_t valid_mutex;
pthread_mutex_t invalid_mutex;


void init ( )
{
    pthread_mutex_init(&valid_mutex, nullptr);
    pthread_mutex_init(&invalid_mutex, nullptr);
}


void destroy ( )
{
    pthread_mutex_destroy(&valid_mutex);
    pthread_mutex_destroy(&invalid_mutex);
}


/* put url to invalid/valid url set */
int processResponse (http::Response & resp, http::Url & url )
{
    if (resp.getStatusCode() != 200) {
        return -1; // TODO provide handler on 301
    }
    
    pthread_mutex_lock(&invalid_mutex);
    pthread_mutex_lock(&valid_mutex);

    int valid = 0;

    if ((invalid_urlset.find(url.getStr()) != invalid_urlset.end()) ||
        (valid_urlset.find(url.getStr()) != valid_urlset.end())) {
        valid = -1; // visited already
    }
    else {
        std::string content_type = resp.getHeader(http::CONTENT_TYPE);
        if (content_type.find(http::CONTENT_TYPE_HTML) == std::string::npos) {
            LOG(DEBUG3) << "skipped response from " << url.getStr() << 
                " . Content-type: " << content_type;
            invalid_urlset.insert(url.getStr());
            valid = -1;
        }
        else {
            valid_urlset.insert(url.getStr());
            valid = 0;
        }
    }

    pthread_mutex_unlock(&valid_mutex);
    pthread_mutex_unlock(&invalid_mutex);
    
    return valid;
}


/* get all links */
int reapLink (http::Response & resp, std::list<std::string> & link_tbl )
{
    const char * body_c = resp.getBody().c_str();
    const char * a_pos, * a_end, * link_pos, * link_end;
    size_t skip_len = 0;
    a_pos = body_c;
    a_end = link_pos = link_end = 0;
    
    while ((a_pos = strcasestr(a_pos, "<a")) != nullptr) {
        a_pos += 2; // prepare for next while loop
        if (nullptr == (a_end = strstr(a_pos, ">"))) {
            continue;
        }
        if (nullptr == (link_pos = strcasestr(a_pos, "href"))) {
            continue;
        }
        link_pos += 4;
        if (0 == (skip_len = strspn(link_pos, " ="))) {
            continue; // potential bug. case "href  '...'" will pass tst
        }
        link_pos += skip_len;
        if (('"' == *link_pos) || ('\'' == *link_pos)) {
            link_pos++;
            link_end = strchr(link_pos, *(link_pos - 1)); 
        }
        else {
            link_end = strpbrk(link_pos, " >\t\r\n");
        }
        const char * anchor_pos = strchr(link_pos, '#');
        if ((anchor_pos != nullptr) && (anchor_pos < link_end)) {
            link_end = anchor_pos;
        }

        if (link_end == nullptr) {
            continue;
        }

        link_tbl.push_back(std::string(link_pos, link_end - link_pos));
        LOG(DEBUG2) << "crawler reaped link: " << link_tbl.back();
    }

    return link_tbl.size();
}


/* iterate links, filter out non-qualified ones, resolve relative ones, dump */
int processLink (http::Url & url, std::list<std::string> & link_tbl )
{
    pthread_mutex_lock(&invalid_mutex);
    pthread_mutex_lock(&valid_mutex);
    
    const std::string scheme = url.getScheme();
    const std::string host = url.getHost();
    const std::string path = url.getPath();
    std::list<std::string>::iterator it = link_tbl.begin(); 
    for ( ; it != link_tbl.end(); ) {
        std::string link = *it;
        if ((invalid_urlset.find(*it) != invalid_urlset.end()) ||
            (valid_urlset.find(*it) != valid_urlset.end())) {
            LOG(DEBUG2) << "link " << link << " is visited";
            it = link_tbl.erase(it);
        }
        else if ((http::validateUrl(*it) != 0) ||
                 (http::normalizeUrl(*it) != 0) ||
                 (http::resolveUrl(*it, scheme, host, path) != 0)) {
            invalid_urlset.insert(link);
            LOG(DEBUG2) << "link " << link << " is invalid";
            it = link_tbl.erase(it);
        }
        else {
            valid_urlset.insert(link);
            LOG(DEBUG2) << "link " << link << " is pending";
            it++;
        }
    }

    pthread_mutex_unlock(&valid_mutex);
    pthread_mutex_unlock(&invalid_mutex);
    return link_tbl.size();
}


void downloadText (const std::string & url, const std::string & content )
{
    std::string filename = http::urlToFileName(url);
    std::ofstream ofs_file (g_download_dir + "/" + filename);
    ofs_file << content;
    ofs_file.close();
    LOGnPRINT(INFO) << "downloaded " << url << ". " << content.size() << " bytes"; 
}

};

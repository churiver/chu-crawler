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


// Status change of a url
//          crawl-link     response
// --------------------------------
// url +---> invalid
//     |---> pending +---> invalid
//                   |---> valid
//                   |---> (already in invalid/valid urlset)
std::set<std::string> n_invalid_urlset;
std::set<std::string> n_pending_urlset;
std::set<std::string> n_valid_urlset;
pthread_mutex_t urlset_mutex;


void init ( )
{
    pthread_mutex_init(&urlset_mutex, nullptr);
}


void destroy ( )
{
    pthread_mutex_destroy(&urlset_mutex);
}


/* put url to invalid/valid url set */
int processResponse (http::Response & resp, http::Url & url )
{
    if (resp.getStatusCode() != 200) {
        LOG(DEBUG3) << "skipped response from " << url.getStr() <<
             " : status code = " << resp.getStatusCode();
        return -1; // TODO provide handler on 301
    }
    
    pthread_mutex_lock(&urlset_mutex);
    
    if (n_pending_urlset.find(url.getStr()) != n_pending_urlset.end()) {
        LOG(DEBUG2) << "remove " << url.getStr() << " from pending urlset";
        n_pending_urlset.erase(url.getStr());
    }

    int valid = 0;
    
    if ((n_invalid_urlset.find(url.getStr()) != n_invalid_urlset.end()) ||
        (n_valid_urlset.find(url.getStr()) != n_valid_urlset.end())) {
        valid = -2; // visited already
    }
    else {
        std::string content_type = resp.getHeader(http::CONTENT_TYPE);
        if (content_type.find(http::CONTENT_TYPE_HTML) == std::string::npos) {
            LOG(DEBUG3) << "skipped response from " << url.getStr() << 
                " : Content-type = " << content_type;
            n_invalid_urlset.insert(url.getStr());
            valid = -3;
        }
        else {
            LOG(DEBUG2) << "insert " << url.getStr() << " into valid urlset";
            n_valid_urlset.insert(url.getStr());
            valid = 0;
        }
    }

    pthread_mutex_unlock(&urlset_mutex);
    
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
    pthread_mutex_lock(&urlset_mutex);

    const std::string scheme = url.getScheme();
    const std::string host = url.getHost();
    const std::string path = url.getPath();
    std::list<std::string>::iterator it = link_tbl.begin(); 
    for ( ; it != link_tbl.end(); ) {
        std::string link = *it;
        if ((n_valid_urlset.find(*it) != n_valid_urlset.end()) ||
            (n_invalid_urlset.find(*it) != n_invalid_urlset.end()) || 
            (n_pending_urlset.find(*it) != n_pending_urlset.end())) {
            LOG(DEBUG2) << "link " << link << " is known";
            it = link_tbl.erase(it);
        }
        else if ((http::validateUrl(*it) != 0) ||
                 (http::normalizeUrl(*it) != 0) ||
                 (http::resolveUrl(*it, scheme, host, path) != 0)) {
            n_invalid_urlset.insert(link);
            LOG(DEBUG2) << "link " << link << " is invalid";
            it = link_tbl.erase(it);
        }
        else {
            n_pending_urlset.insert(link);
            LOG(DEBUG2) << "link " << link << " is pending";
            it++;
        }
    }

    pthread_mutex_unlock(&urlset_mutex);
    return link_tbl.size();
}


int downloadText (const std::string & url, const std::string & content, int dnldcount )
{
    std::string filename = http::urlToFileName(url);
    std::ofstream ofs_file (g_download_dir + "/" + std::to_string(dnldcount+1) + "-" + filename);
    //LOGnPRINT(INFO) << filename << " is_open = " << ofs_file.is_open() 
    //        << ", rdstate = " << ofs_file.rdstate() << ", eof = " << ofs_file.eof()
    //        << ", fail = " << ofs_file.fail() << ", bad = " << ofs_file.bad();
    if (ofs_file.is_open() != true) {
        LOGnPRINT(WARNING) << "download failed. reason: " << strerror(errno);
        return -1;
    }
    ofs_file << content;
    ofs_file.close();
    LOGnPRINT(INFO) << "downloaded " << url << ". " << content.size() << " bytes"; 
    return 0;
}

};

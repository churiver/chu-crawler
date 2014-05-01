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
#include "httpResponse.h"
#include "transportUtil.h"

extern std::string g_download_dir;

namespace crawl {


std::set<std::string> valid_urlset;
std::set<std::string> invalid_urlset;
pthread_mutex_t valid_mutex;
pthread_mutex_t invalid_mutex;

#define MAX_FILENAME_LEN 64


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

    std::string content_type = resp.getHeader(http::CONTENT_TYPE);
    if (content_type.find(http::CONTENT_TYPE_HTML) == std::string::npos) {
        LOG(DEBUG3) << "skipped non-webpage response from " << url.getStr();
        //fprintf(stdout, "%s is not webpage and is skipped\n", url.getStr());
        invalid_urlset.insert(url.getStr());
        valid = -1;
    }
    else {
        valid_urlset.insert(url.getStr());
        valid = 0;
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

        if (link_end == nullptr) {
            continue;
        }

        link_tbl.push_back(std::string(link_pos, link_end - link_pos));
        LOG(DEBUG2) << "crawler reaped link: " << link_tbl.back();
        //fprintf(stderr, "crawler reaped a link: %s\n", link_tbl.back().c_str());
    }

    return link_tbl.size();
}


/* iterate links, filter out non-qualified ones, resolve relative ones, dump */
int processLink (http::Url & url, std::list<std::string> & link_tbl )
{
    //fprintf(stdout, "%d crawlUrl: locking invalid_url_mutex\n", pthread_self());
    pthread_mutex_lock(&invalid_mutex);
    //fprintf(stdout, "%d crawlUrl: locked invalid_url_mutex\n", pthread_self());
    //fprintf(stdout, "%d crawlUrl: locking valid_url_mutex\n", pthread_self());
    pthread_mutex_lock(&valid_mutex);
    //fprintf(stdout, "%d crawlUrl: locked valid_url_mutex\n", pthread_self());
    
    const std::string host = url.getHost();
    const std::string path = url.getPath();
    std::list<std::string>::iterator it = link_tbl.begin(); 
    for ( ; it != link_tbl.end(); ) {
        std::string link = *it;
        if ((invalid_urlset.find(*it) != invalid_urlset.end()) ||
            (valid_urlset.find(*it) != valid_urlset.end())) {
            LOG(DEBUG2) << "link " << link << " is visited";
            //fprintf(stderr, "link %s is visited\n", link.c_str());
            it = link_tbl.erase(it);
        }
        else if ((http::validateUrl(*it) != 0) ||
                 (http::normalizeUrl(*it) != 0) ||
                 (http::resolveUrl(*it, host, path) != 0)) {
            invalid_urlset.insert(link);
            LOG(DEBUG2) << "link " << link << " is invalid";
            //fprintf(stderr, "link %s is invalid\n", link.c_str());
            it = link_tbl.erase(it);
        }
        else {
            valid_urlset.insert(link);
            LOG(DEBUG2) << "link " << link << " is pending";
            //fprintf(stderr, "link %s is pending\n", link.c_str());
            it++;
        }
    }

    //fprintf(stdout, "%d crawlUrl: unlocking invalid_url_mutex\n", pthread_self());
    pthread_mutex_unlock(&valid_mutex);
    //fprintf(stdout, "%d crawlUrl: unlocked invalid_url_mutex\n", pthread_self());
    //fprintf(stdout, "%d crawlUrl: unlocking valid_url_mutex\n", pthread_self());
    pthread_mutex_unlock(&invalid_mutex);
    //fprintf(stdout, "%d crawlUrl: unlocked valid_url_mutex\n", pthread_self());
    return link_tbl.size();
}


// MOVE TO httpUtil
const std::string urlToFileName (std::string urlstr )
{
    std::size_t protocol = urlstr.find("://");
    std::size_t last_slash = urlstr.find_last_of('/');
    std::size_t last_dot = urlstr.find_last_of('.');
    
    if ((std::string::npos == last_slash) || // www.abc.com
        (protocol == (last_slash - 2)) ||    // http://www.abc.com
        (last_dot < last_slash)) {           // http://www.abc.com
        urlstr += ".html";
    }

    std::string::iterator it = urlstr.begin();
    for (it; it < urlstr.end(); it++) {
        if (*it == '/' || *it == '\\' || *it == '?' || 
            *it == ':' || *it == '*' || *it == '|' ||
            *it == '<' || *it == '>' || *it == '"' || 
                *it == '\'') {
            *it = '_';
        }        
    }
    return urlstr;
}


const char * urlToFileName (const char * urlstr )
{
//  regex throw regex_error
//    std::regex expr("[\\?/:*|<>\"']");
//    std::string result = std::regex_replace(urlstr, expre, "_");
//    return result;

    const char * url_ptr = strstr(urlstr, "://");
    url_ptr = (nullptr == url_ptr) ? urlstr : (url_ptr + 3);
    
    size_t name_len = strlen(url_ptr);
    name_len = (name_len < MAX_FILENAME_LEN) ? name_len : MAX_FILENAME_LEN;
    char * name = new char[name_len + 6]();
    char * name_ptr = name;
    
    for (int i = 0; i < name_len; i++) {
        if (*url_ptr == '/' || *url_ptr == '\\' ||
                *url_ptr == '?' || *url_ptr == ':' ||
                *url_ptr == '*' || *url_ptr == '|' ||
                *url_ptr == '<' || *url_ptr == '>' ||
                *url_ptr == '"' || *url_ptr == '\'') {
            *name_ptr = '_';
        }
        else {
            *name_ptr = *url_ptr;
        }
        name_ptr++;
        url_ptr++;
    }
    *name_ptr = '\0';

    // TODO handle case of url contains .html or other postfix already
    return strcat(name, ".html");
}


void downloadText (const std::string & url, const std::string & content )
{
    std::string filename = urlToFileName(url);
    std::ofstream ofs_file (g_download_dir + "/" + urlToFileName(url));
    ofs_file << content;
    ofs_file.close();
    LOG(DEBUG3) << "downloaded " << url; 
}

};

/**
* Copyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver at gmail.com
* @date 03/04/2014
* @description Crawler
*/

#include "Worker.h"

#include <cstring>
#include <cstdio>

#include "modules/util.h"


// WARNING
// inline static member initialization creates a scoped variable
// need a out-scoped definition
std::set<std::string> Worker::known_url_set;
std::set<std::string> Worker::bad_url_set;
 

Worker::Worker (int id_ = 1 )
{
    id = id_;
}


Worker::~Worker ( ) {}


void Worker::start (const char * seed_url )
{
    if ((seed_url == NULL) || (strlen(seed_url) == 0))
        return;

    pending_url_que.push(std::string(seed_url));
    while (!pending_url_que.empty() && 
           (bad_url_set.size() + known_url_set.size() < 100)) {

        std::string next_url = pending_url_que.front();
        pending_url_que.pop();

        int link_sum = crawlUrl(next_url);
        printf("Worker crawled %d links on url %s\n\n", link_sum, next_url.c_str());
    }

    printf("Worker finished work with %d visited urls and %d bad urls\n", 
                known_url_set.size(), bad_url_set.size());

}


int Worker::crawlUrl (std::string & url )
{
    if (http::normalizeUrl(url) != 0) {
        bad_url_set.insert(url);
        return -1;
    }

    http::Request req(url, http::Request::METHOD_GET);
    req.setHeader(http::Request::HEADER_CONN, "close");
    req.setHeader(http::Request::HEADER_AGENT, "tiny-crawler");
    
    http::State state;
    if ((state = req.execute()) != http::State::OK) {
        printf("Worker crawled url failed, state %d\n", state);
        bad_url_set.insert(url);
        return -1;
    }

    known_url_set.insert(url);
    http::Response & resp = req.getResponse();

    std::string content_type = resp.getHeader(http::Response::CONTENT_TYPE);
    if (content_type.find(http::Response::CONTENT_TYPE_HTML) == std::string::npos) {
        printf("Worker found a non-html link: %s, type %s\n", url.c_str(), 
            content_type.c_str());
        return -1;
    }

    std::string path("download");
    resp.downloadFile(url, path); // TODO read path from conf

    std::list<std::string> link_table;
    int total_link_num = reapLinks(resp, link_table);
    if (total_link_num == -1) {
        printf("Worker reaped none link due to ineligible response\n");
        return -1;
    }
    printf("Worker reaped %d links\n", total_link_num);

    int valid_link_num = processLinks(req, link_table);
    printf("Worker processed out %d links\n", total_link_num - valid_link_num);

    return valid_link_num;
}


/**
 * get all links
 */
int Worker::reapLinks (http::Response & resp, std::list<std::string> & link_table )
{
    if (resp.getStatusCode() != 200)
        return -1; // TODO provide handler on 301
    
    const char * body_c = resp.getBody().c_str();
    const char * a_pos, * a_end, * link_pos, * link_end;
    size_t skip_len = 0;
    a_pos = body_c;
    a_end = link_pos = link_end = 0;

    
    while ((a_pos = strcasestr(a_pos, "<a")) != NULL) {
        a_pos += 2; // prepare for next while loop
        if ((a_end = strstr(a_pos, ">")) == NULL)
            continue;
        if ((link_pos = strcasestr(a_pos, "href")) == NULL)
            continue;
        link_pos += 4;
        if ((skip_len = strspn(link_pos, " =")) == 0)
            continue; // potential bug. case "href  '...'" will pass tst
        link_pos += skip_len;
        if ((*link_pos == '"') || (*link_pos == '\'')) {
            link_pos++;
            link_end = strchr(link_pos, *(link_pos - 1)); 
            // C++ does npt promise execution sequence of expr:
            //   strchr(++link_pos, *link_pos))
            // in my test *link_pos take old value before ++.
        }
        else 
            link_end = strpbrk(link_pos, " >\t\r\n");

        if (link_end == NULL)
            continue;

        link_table.push_back(std::string(link_pos, link_end - link_pos));
        printf("Worker reaped a link: %s\n", link_table.back().c_str());
    }
    return link_table.size();
}


/**
 * iterate links, filter out non-qualified ones, resolve relative ones, dump
 */
int Worker::processLinks (http::Request & req, std::list<std::string> & link_table )
{
    int pending_num = 0;
    const std::string base_host = req.getBaseHost();
    const std::string path = req.getPath();
    std::list<std::string>::iterator it = link_table.begin(); 
    for (it; it != link_table.end(); it++) {
        if ((bad_url_set.find(*it) != bad_url_set.end()) ||
                (known_url_set.find(*it) != known_url_set.end())) {
            printf("link %s is known\n", (*it).c_str());
            continue;
        }

        if ((http::checkUrlType(*it) == 1) ||
                (http::normalizeUrl(*it) == 1)) {
            bad_url_set.insert(*it);
            printf("link %s is bad\n", (*it).c_str());
            continue;
        }

        http::resolveUrl(*it, base_host, path);
        printf("link %s is pending\n", (*it).c_str());
        pending_url_que.push(*it);
        pending_num++;
    }
    return pending_num;
}


int Worker::checkResponse (http::Response & response )
{
    // TODO check resp header 
    //   discard content-type other than html
    //   handle status code other than 200
}


int main(int argc, char ** argv)
{
    if (argc != 2) {
        printf("Usage: ./chu-crawler [url]\n");
        printf("url format: http://[host]/[path]\n");
        return 1;    
    }

    Worker worker(1);
    worker.start(argv[1]);
    return 0;
}

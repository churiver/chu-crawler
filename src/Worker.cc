/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver at gmail.com
* @date 03/04/2014
* @description Crawler
*/

#include "Worker.h"

#include <cstring>
#include <cstdio>
#include <pthread.h>

#include "modules/util.h"


// WARNING
// inline static member initialization creates a scoped variable
// need a out-scoped definition
std::set<std::string> Worker::known_url_set;
std::set<std::string> Worker::bad_url_set;
pthread_mutex_t Worker::known_url_mutex;
pthread_mutex_t Worker::bad_url_mutex;
pthread_mutex_t Worker::download_url_mutex;
unsigned int Worker::instance_count = 0;
unsigned int Worker::next_id = 1;
unsigned int Worker::download_url_count = 0;
unsigned int Worker::max_url_count;
std::string Worker::download_path;


Worker::Worker ( )
{
    id = next_id++;
    url_count = 0;
    if (instance_count++ == 0) {
        pthread_mutex_init(&bad_url_mutex, nullptr);
        pthread_mutex_init(&known_url_mutex, nullptr);
    }
}


Worker::~Worker ( ) 
{
    if (--instance_count == 0) {
        pthread_mutex_destroy(&bad_url_mutex);
        pthread_mutex_destroy(&known_url_mutex);
    }
}


void Worker::start (const char * seed_url )
{
    if ((seed_url == nullptr) || (strlen(seed_url) == 0))
        return;

    pending_url_que.push(std::string(seed_url));
    while (!pending_url_que.empty() && !isTargetAchieved()) {

        std::string next_url = pending_url_que.front();
        pending_url_que.pop();

        fprintf(stdout, "== Worker start crawling url %s\n", next_url.c_str());
        int link_count = crawlUrl(next_url);
        fprintf(stdout, "== Worker got %d links on url %s\n\n", 
                    link_count, next_url.c_str());

        // Unlock the mutex in case it return from exception
        if (link_count == -1) {
            //fprintf(stdout, "%d start: unlocking mutex\n", pthread_self());
            pthread_mutex_unlock(&known_url_mutex);
            pthread_mutex_unlock(&bad_url_mutex);
            //fprintf(stdout, "%d start: unlocked mutex\n", pthread_self());
        }
    }

    fprintf(stdout, "Worker thread %d done. Downloaded %d urls\n", 
                pthread_self(), url_count);
}


void Worker::start (const std::string seed_url )
{
    start(seed_url.c_str());
}


int Worker::crawlUrl (std::string & url )
{
    //fprintf(stdout, "%d crawlUrl: locking bad_url_mutex\n", pthread_self());
    pthread_mutex_lock(&bad_url_mutex);
    //fprintf(stdout, "%d crawlUrl: locked bad_url_mutex\n", pthread_self());
    //fprintf(stdout, "%d crawlUrl: locking known_url_mutex\n", pthread_self());
    pthread_mutex_lock(&known_url_mutex);
    //fprintf(stdout, "%d crawlUrl: locked known_url_mutex\n", pthread_self());
    
    if (http::normalizeUrl(url) != 0) {
        bad_url_set.insert(url);
        return -1;
    }

    http::Request req(url, http::Request::METHOD_GET);
    req.setHeader(http::Request::HEADER_CONN, "close");
    req.setHeader(http::Request::HEADER_AGENT, "tiny-crawler");
    
    http::State state;
    if ((state = req.execute()) != http::State::OK) {
        fprintf(stdout, "Worker crawled url failed, state %d\n", state);
        bad_url_set.insert(url);
        return -1;
    }
    known_url_set.insert(url);

    //fprintf(stdout, "%d crawlUrl: unlocking bad_url_mutex\n", pthread_self());
    pthread_mutex_unlock(&known_url_mutex);
    //fprintf(stdout, "%d crawlUrl: unlocked bad_url_mutex\n", pthread_self());
    //fprintf(stdout, "%d crawlUrl: unlocking known_url_mutex\n", pthread_self());
    pthread_mutex_unlock(&bad_url_mutex);
    //fprintf(stdout, "%d crawlUrl: unlocked known_url_mutex\n", pthread_self());

    http::Response & resp = req.getResponse();

    std::string content_type = resp.getHeader(http::Response::CONTENT_TYPE);
    if (content_type.find(http::Response::CONTENT_TYPE_HTML) == std::string::npos) {
        fprintf(stdout, "Worker found a non-html link: %s, type %s\n", url.c_str(), 
            content_type.c_str());
        return -1;
    }

    resp.downloadFile(url, download_path);
    url_count++;

    //fprintf(stdout, "%d crawlUrl: locking download_url_mutex\n", pthread_self());
    pthread_mutex_lock(&download_url_mutex);
    download_url_count++;
    pthread_mutex_unlock(&download_url_mutex);
    //fprintf(stdout, "%d crawlUrl: unlocked download_url_mutex\n", pthread_self());

    std::list<std::string> link_table;
    int total_link_count = reapLinks(resp, link_table);
    if (total_link_count == -1) {
        fprintf(stdout, "Worker reaped none link due to ineligible response\n");
        return -1;
    }
    fprintf(stdout, "Worker reaped %d links\n", total_link_count);

    int valid_link_count = processLinks(req, link_table);
    fprintf(stdout, "Worker processed out %d links\n", 
                total_link_count - valid_link_count);

    return valid_link_count;
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

    
    while ((a_pos = strcasestr(a_pos, "<a")) != nullptr) {
        a_pos += 2; // prepare for next while loop
        if ((a_end = strstr(a_pos, ">")) == nullptr)
            continue;
        if ((link_pos = strcasestr(a_pos, "href")) == nullptr)
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

        if (link_end == nullptr)
            continue;

        link_table.push_back(std::string(link_pos, link_end - link_pos));
        fprintf(stderr, "Worker reaped a link: %s\n", link_table.back().c_str());
    }
    return link_table.size();
}


/**
 * iterate links, filter out non-qualified ones, resolve relative ones, dump
 */
int Worker::processLinks (http::Request & req, std::list<std::string> & link_table )
{
    //fprintf(stdout, "%d crawlUrl: locking bad_url_mutex\n", pthread_self());
    pthread_mutex_lock(&bad_url_mutex);
    //fprintf(stdout, "%d crawlUrl: locked bad_url_mutex\n", pthread_self());
    //fprintf(stdout, "%d crawlUrl: locking known_url_mutex\n", pthread_self());
    pthread_mutex_lock(&known_url_mutex);
    //fprintf(stdout, "%d crawlUrl: locked known_url_mutex\n", pthread_self());
    
    int valid_link_count = 0;
    const std::string base_host = req.getBaseHost();
    const std::string path = req.getPath();
    std::list<std::string>::iterator it = link_table.begin(); 
    for (it; it != link_table.end(); it++) {
        if ((bad_url_set.find(*it) != bad_url_set.end()) ||
                (known_url_set.find(*it) != known_url_set.end())) {
            fprintf(stderr, "link %s is known\n", (*it).c_str());
            continue;
        }

        if ((http::checkUrlType(*it) == 1) ||
                (http::normalizeUrl(*it) == 1)) {
            bad_url_set.insert(*it);
            fprintf(stderr, "link %s is bad\n", (*it).c_str());
            continue;
        }

        http::resolveUrl(*it, base_host, path);
        fprintf(stderr, "link %s is pending\n", (*it).c_str());
        pending_url_que.push(*it);
        valid_link_count++;
    }

    //fprintf(stdout, "%d crawlUrl: unlocking bad_url_mutex\n", pthread_self());
    pthread_mutex_unlock(&known_url_mutex);
    //fprintf(stdout, "%d crawlUrl: unlocked bad_url_mutex\n", pthread_self());
    //fprintf(stdout, "%d crawlUrl: unlocking known_url_mutex\n", pthread_self());
    pthread_mutex_unlock(&bad_url_mutex);
    //fprintf(stdout, "%d crawlUrl: unlocked known_url_mutex\n", pthread_self());

    return valid_link_count;
}


int Worker::checkResponse (http::Response & response )
{
    // TODO check resp header 
    //   discard content-type other than html
    //   handle status code other than 200
}


bool Worker::isTargetAchieved( )
{
    //fprintf(stdout, "%d isTarget: locking download_url_mutex\n", pthread_self());
    pthread_mutex_lock(&download_url_mutex);
    bool ret = (download_url_count < max_url_count) ? false : true;
    pthread_mutex_unlock(&download_url_mutex);
    //fprintf(stdout, "%d isTarget: unlocked download_url_mutex\n", pthread_self());
    return ret;
}


/*
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
}*/

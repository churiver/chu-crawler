/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/16/2014
* @description crawler
*/

#include "crawler.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <strings.h>

#include <sys/epoll.h>
#include <errno.h>

#include <iostream>
#include <list>
#include <fstream>

#include "lib/Logger.h"
#include "transportUtil.h"
#include "httpUtil.h"
#include "httpUrl.h"
#include "httpResponse.h"
#include "crawlUtil.h"


int main ( )
{
    if (0 != init()) {
        return -1;
    }

    int nfds, n;
    int epollfd = transport::getEpollfd();
    struct epoll_event ev, events[MAX_EPOLL_EVENTS];

    // initial crawler with seeds
    std::vector<std::string>::iterator it = g_seeds.begin();
    for ( ; it != g_seeds.end(); it++) {
        thread::Task * task = 
            new thread::Task(handleUrl, (void *)strdup((*it).c_str()));
        g_urltask_pool.addTask(task);
    }

    while (g_current_dnldcount < g_target_dnldcount) {

        do {
            nfds = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);
            LOG(INFO) << "epoll_wait() returns nfds " << nfds << ", errno " << errno;
        } while ((nfds < 0) && (EINTR == errno));

        for (int i = 0; i < nfds; i++) {
            struct transport::FdInfo * fdinfo = 
                (struct transport::FdInfo *)events[i].data.ptr;

            LOG(INFO) << "events[" << i << "] event " << 
                    events[i].events << ", fd " << fdinfo->fd;

            int op = EPOLL_CTL_DEL;
            int sockfd = fdinfo->fd;
            int ret = 0;

            if ((events[i].events & EPOLLIN)) {
                if ((ret = transport::recv(fdinfo)) != -1) {
                    thread::Task * task = 
                        new thread::Task(handleResponse, fdinfo);
                    g_responsetask_pool.addTask(task);
                    close(sockfd);
                }
                // handle uncomplete read (move out if want handle write as well)
                else if ((-1 == ret) && (EAGAIN == errno)) {
                    events[i].events = events[i].events | EPOLLONESHOT;
                    op = EPOLL_CTL_MOD;
                    if (-1 == epoll_ctl(epollfd, op, fdinfo->fd, &events[i])) {
                        LOG(ERROR) << "epoll_ctl() failed. errno " << errno;
                        return -1;
                    }
                }
            }
            else if ((events[i].events & EPOLLERR) ||
                     (events[i].events & EPOLLHUP)) {

                LOG(INFO) << "closing fd " << fdinfo->fd;
                transport::freeFdInfo(fdinfo);
                close(sockfd); // close() will remove fd from epoll set
            }
        }
    }

    destroy();
    return 0;
}


int init( )
{
    std::map<std::string, std::string> conf_map;
    std::ifstream configfile(CONFIG_FILE);
    if (!configfile.is_open()) {
        std::cerr <<  "Cant open config file " << CONFIG_FILE;
        return -1;
    }

    std::string line;
    size_t delim_pos;
    while (getline(configfile, line)) {
        if ((line.empty()) || ('#' == line[0]) ||
            ((delim_pos = line.find('=')) == std::string::npos)) {
            continue;
        }
        std::string prop(line.substr(0, delim_pos));
        std::string value(line.substr(delim_pos + 1));
        if (prop == CONF_SEED) {
            g_seeds.push_back(value);
        }
        else {
            conf_map[prop] = value;
        }
    }

    if (!conf_map[CONF_DOWNLOAD_DIR].empty()) {
        g_download_dir = conf_map[CONF_DOWNLOAD_DIR];
    }
    int tmp_count = atoi(conf_map[CONF_TARGET_DNLDCOUNT].c_str());
    if (tmp_count != 0) {
        g_target_dnldcount = tmp_count;
    }

    logs::Logger::init(
            conf_map[CONF_LOG_FILE], 
            conf_map[CONF_LOG_MIN_LEVEL],
            atoi(conf_map[CONF_LOG_LOW_WATERMARK].c_str())
            );

    crawl::init();

    return 0;
}


int destroy( )
{
    g_urltask_pool.destroy();
    g_responsetask_pool.destroy();
    crawl::destroy();
    logs::Logger::destroy();
}


void handleUrl(void * arg )
{
    LOG(INFO) << "entering task..";
    http::Url url((char *)arg);
    free(arg);
    if (url.getState() != http::OK) {
        LOG(WARNING) << "parsing url failed: " << url.getState();
        return;
    }

    int sockfd = transport::connectTo(url.getIp(), url.getPort());
    char * req_msg = http::setRequest(url.getHost(), url.getPath());

    struct transport::FdInfo * fdinfo = (struct transport::FdInfo *)calloc(1,
                                            sizeof(struct transport::FdInfo));
    fdinfo->fd = sockfd;
    fdinfo->uri = strdup(url.getStr().c_str()); // used in handleResponse
    fdinfo->buff = req_msg;

    int ret = transport::send(fdinfo);

    if  (-1 == ret) {
        LOG(WARNING) << "send request failed. errno " << errno;
        transport::freeFdInfo(fdinfo);
        close(sockfd);
        return;
    }
    else {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.ptr = fdinfo;
        free(fdinfo->buff);
        fdinfo->buff = nullptr; // forgot change buff to NULL resulted overflow!
        epoll_ctl(transport::getEpollfd(), EPOLL_CTL_ADD, sockfd, &ev);
    }
}


void handleResponse(void * arg )
{
    LOG(INFO) << "entering task..";
    struct transport::FdInfo * fdinfo = (struct transport::FdInfo *)arg;
    http::Url url(fdinfo->uri, true);
    http::Response resp(fdinfo->buff);
    transport::freeFdInfo(fdinfo);

    if ((crawl::processResponse(resp, url)) != 0) {
        return;
    }
    crawl::downloadText(url.getStr(), resp.getBody());
    g_current_dnldcount++;

    std::list<std::string> link_tbl;
    int total_links = crawl::reapLink(resp, link_tbl);
    LOG(INFO) << "crawler reaped " << total_links << " links";
    if (total_links <= 0) {
        return;
    }
    
    int valid_links = crawl::processLink(url, link_tbl);
    LOG(INFO) << "crawler filtered out " << total_links - valid_links << " links";

    std::list<std::string>::iterator it = link_tbl.begin();
    for ( ; it != link_tbl.end(); it++) {
        thread::Task * task = new thread::Task(handleUrl, (void *)strdup((*it).c_str()));
        g_urltask_pool.addTask(task);
    }
}


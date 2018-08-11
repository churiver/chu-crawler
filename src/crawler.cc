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

#include <errno.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>

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
    if (init() != 0) {
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

    while (g_target_done != true) {
        // epoll_wait maybe interrupted before events come so wrapped in while loop
        do {
            nfds = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, 5000);
            LOG(DEBUG3) << "epoll_wait() returns " << nfds << " nfds. errno " 
                << errno;
        } while ((nfds < 0) && (EINTR == errno));

        for (int i = 0; i < nfds; i++) {
            struct transport::FdInfo * fdinfo = 
                (struct transport::FdInfo *)events[i].data.ptr;

            LOG(DEBUG3) << "events[" << i << "] event " << 
                    events[i].events << ", fd " << fdinfo->fd;

            int op = EPOLL_CTL_DEL;
            int sockfd = fdinfo->fd;
            int ret = 0;

            if ((events[i].events & EPOLLIN)) {
                // read complete
                if ((ret = transport::recv(fdinfo)) != -1) {
                    thread::Task * task = 
                        new thread::Task(handleResponse, fdinfo);
                    g_resptask_pool.addTask(task);
                    LOG(DEBUG3) << "Receive complete. closing fd " << sockfd;
                    close(sockfd);
                    g_connection_queue.take();
                    LOG(DEBUG3) << "active connection queue decrease to " << 
                        g_connection_queue.getSize();
                }
                // read uncomplete
                else if ((-1 == ret) && (EAGAIN == errno)) {
                    events[i].events = events[i].events | EPOLLONESHOT;
                    op = EPOLL_CTL_MOD;
                    if (-1 == epoll_ctl(epollfd, op, fdinfo->fd, &events[i])) {
                        LOG(ERROR) << "epoll_ctl() failed. reason " << 
                            strerror(errno);
                        return -1;
                    }
                }
            }
            else if ((events[i].events & EPOLLERR) ||
                     (events[i].events & EPOLLHUP)) {
                transport::freeFdInfo(fdinfo);
                LOG(DEBUG3) << "Receive failed. closing fd " << sockfd;
                close(sockfd); // close() will remove fd from epoll set
                g_connection_queue.take();
                LOG(DEBUG3) << "active connection queue decrease to " << 
                    g_connection_queue.getSize();
            }
            else {
                LOG(DEBUG3) << "Receive unhandlable event";
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
        std::cerr << "Cant open config file " << CONFIG_FILE;
        return -1;
    }

    // read config file
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
	const int cmd_ret = system(("mkdir -p " + g_download_dir).c_str());
	if (cmd_ret != 0) {
            std::cerr << "Cannot create download dir " << g_download_dir;
            return -1;
        }
    }
    int tmp_value = 0;
    if ((tmp_value = atoi(conf_map[CONF_TARGET_DNLDCOUNT].c_str())) != 0) {
        g_target_dnldcount = tmp_value;
    }
    if ((tmp_value = atoi(conf_map[CONF_URLTASK_POOLSIZE].c_str())) != 0) {
        g_urltask_pool.increaseSizeTo(tmp_value);
    }
    if ((tmp_value = atoi(conf_map[CONF_RESPTASK_POOLSIZE].c_str())) != 0) {
        g_resptask_pool.increaseSizeTo(tmp_value);
    }
    g_urltask_pool.setPriority(LOW_PRIORITY);

    // init other resources
    logs::Logger::init(
            conf_map[CONF_LOG_FILE], 
            conf_map[CONF_LOG_MIN_LEVEL],
            atoi(conf_map[CONF_LOG_LOW_WATERMARK].c_str())
            );
    crawl::init();
    pthread_mutex_init(&g_target_mutex, nullptr);
    pthread_create(&g_urldaemon_id, nullptr, urlDaemon, nullptr);

    return 0;
}


int destroy( )
{
    LOGnPRINT(INFO) << "Main destroying...";
    g_urltask_pool.destroy();
    g_resptask_pool.destroy();
    crawl::destroy();
    pthread_mutex_destroy(&g_target_mutex);
    LOGnPRINT(INFO) << "Main destroyed.";
    logs::Logger::destroy();
    pthread_join(g_urldaemon_id, nullptr);
}


void handleUrl(void * arg )
{
    LOG(DEBUG3) << "entering task..";
    http::Url url((char *)arg);
    free(arg);
    if (url.getState() != http::OK) {
        LOG(WARNING) << "parsing url " << url.getStr() << " failed, state: " 
            << url.getState();
        return;
    }

    // connect and send request
    LOG(DEBUG3) << "connect to url " << url.getStr() << ":" << url.getPort();
    int sockfd = transport::connectTo(url.getHost(), url.getPort());
    if (-1 == sockfd) {
        LOG(WARNING) << "connect to url " << url.getStr() << " failed.";
        return;
    }

    char * req_msg = http::setRequest(url.getHost(), url.getPath());
    if (nullptr == req_msg) {
        LOG(WARNING) << "request header is too long";
        close(sockfd);
        return;
    }

    // fdinfo object created here
    struct transport::FdInfo * fdinfo = 
            (struct transport::FdInfo *)calloc(1, sizeof(struct transport::FdInfo));
    fdinfo->fd = sockfd;
    fdinfo->uri = strdup(url.getStr().c_str()); // used in handleResponse
    fdinfo->buff = req_msg;

    int ret = transport::send(fdinfo);

    // if sending fails then return, else reuse fdinfo for coming response
    if  (-1 == ret) {
        LOG(WARNING) << "send request failed. reason " << strerror(errno);
        transport::freeFdInfo(fdinfo);
        close(sockfd);
        return;
    }
    else {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.ptr = fdinfo;
        free(fdinfo->buff);
        fdinfo->buff = nullptr; // not setting buff to NULL will result overflow
        epoll_ctl(transport::getEpollfd(), EPOLL_CTL_ADD, sockfd, &ev);

        g_connection_queue.put('0');
        LOG(DEBUG3) << "active connection queue increase to " << 
            g_connection_queue.getSize();
    }
}


void handleResponse(void * arg )
{
    LOG(INFO) << "entering task..";
    struct transport::FdInfo * fdinfo = (struct transport::FdInfo *)arg;
    http::Url url(fdinfo->uri);
    http::Response resp(fdinfo->buff);
    transport::freeFdInfo(fdinfo);

    LOG(DEBUG3) << "handle response from url " << url.getStr();

    int valid = 0; 
    if ((valid = crawl::processResponse(resp, url)) != 0) {
        LOG(WARNING) << "process response failed: " << valid;
        return;
    }

    // download response
    pthread_mutex_lock(&g_target_mutex);
    if (g_current_dnldcount >= g_target_dnldcount) {
        g_target_done = true;
    }
    else if (0 == crawl::downloadText(
                    url.getStr(), resp.getBody(), g_current_dnldcount)) {
        g_current_dnldcount++;
        LOGnPRINT(INFO) << "current download count: " << g_current_dnldcount;
        if (g_current_dnldcount >= g_target_dnldcount) {
            g_target_done = true;
        }
    }
    pthread_mutex_unlock(&g_target_mutex);
    if (true == g_target_done) {
        return;
    }

    // crawl links
    std::list<std::string> link_tbl;
    int total_links = crawl::reapLink(resp, link_tbl);
    LOG(INFO) << "crawler reaped " << total_links << " links";
    if (total_links <= 0) {
        return;
    }
    
    int valid_links = crawl::processLink(url, link_tbl);
    LOG(INFO) << "crawler filtered out " << total_links - valid_links << 
        " links";

    std::list<std::string>::iterator it = link_tbl.begin();
    for ( ; it != link_tbl.end(); it++) {
        g_url_queue.put(*it);
    }
}


void * urlDaemon(void * arg)
{
    struct rlimit limit;
    int nofile = DEFAULT_NOFILE;
    if (getrlimit(RLIMIT_NOFILE, &limit) != 0) {
        LOG(WARNING) << "Failed to get RLIMIT_NOFILE. Set to default value";
    }
    else {
        nofile = limit.rlim_cur;
    }
    int fd_limit = (nofile - 4) / 2; // 0,1,2 and one for logger
    LOG(INFO) << "set fd limit to " << fd_limit;
    int max_active_connection = 
            (g_target_dnldcount < fd_limit) ? g_target_dnldcount : fd_limit;
    g_connection_queue.setCapacity(max_active_connection);

    // initially take a reasonable number of url from url queue to urltask pool
    for (int i = 0; i < max_active_connection; i++) {
        const char * urlstr = strdup((g_url_queue.take()).c_str());
        thread::Task * task = new thread::Task(handleUrl, (void *)urlstr);
        g_urltask_pool.addTask(task);
    }

    // monitor the number of active connections and supply urls to urltask pool
    //  if needed
    while (g_target_done != true) {
        sleep(5);
        int available_connection_num = 
                max_active_connection - g_connection_queue.getSize();
        int remaining_target = g_target_dnldcount - g_current_dnldcount;
        int n = (available_connection_num < remaining_target) ? 
                    available_connection_num : remaining_target;
        for(int i = 0; i < n; i++) {
            const char * urlstr = strdup((g_url_queue.take()).c_str());
            thread::Task * task = new thread::Task(handleUrl, (void *)urlstr);
            g_urltask_pool.addTask(task); // take n url to urltask pool
        }
    }
}



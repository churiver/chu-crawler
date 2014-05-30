/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/16/2014
* @description crawler header
*/

#ifndef CRAWLER_H
#define CRAWLER_H

#include <pthread.h>

#include <string>
#include <vector>
#include <map>

#include "lib/ThreadPool.h"

const std::string CONFIG_FILE = "crawler.conf";
const std::string CONF_SEED = "seed";
const std::string CONF_DOWNLOAD_DIR = "download-dir";
const std::string CONF_TARGET_VISITCOUNT = "target-visitcount";
const std::string CONF_TARGET_DNLDCOUNT = "target-dnldcount";
const std::string CONF_URLTASK_POOLSIZE = "urltask-poolsize";
const std::string CONF_RESPTASK_POOLSIZE = "resptask-poolsize";
const std::string CONF_LOG_FILE = "log-file";
const std::string CONF_LOG_MIN_LEVEL = "log-min-level";
const std::string CONF_LOG_LOW_WATERMARK = "log-low-watermark";

const std::string DEFAULT_DOWNLOAD_DIR = "download";
const int DEFAULT_DNLDCOUNT = 100;
const int DEFAULT_NOFILE = 1024;
const int LOW_PRIORITY = 19;

std::string g_download_dir = DEFAULT_DOWNLOAD_DIR;
int g_target_dnldcount = DEFAULT_DNLDCOUNT;
int g_current_dnldcount = 0;
bool g_target_done = false;
pthread_mutex_t g_target_mutex;
pthread_t g_urldaemon_id;

thread::ThreadPool g_urltask_pool;
thread::ThreadPool g_resptask_pool;
thread::BlockingQueue<std::string> g_url_queue; 
thread::BlockingQueue<char> g_connection_queue;
std::vector<std::string> g_seeds;

int init( );

int destroy();

void handleUrl(void * );

void handleResponse(void * );

void * urlDaemon(void * );

#endif

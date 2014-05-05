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
const std::string CONF_LOG_FILE = "log-file";
const std::string CONF_LOG_MIN_LEVEL = "log-min-level";
const std::string CONF_LOG_LOW_WATERMARK = "log-low-watermark";

const std::string DEFAULT_DOWNLOAD_DIR = "download";
const int DEFAULT_DNLDCOUNT = 100;

std::string g_download_dir = DEFAULT_DOWNLOAD_DIR;
int g_target_dnldcount = DEFAULT_DNLDCOUNT;
int g_current_dnldcount = 0;
bool g_target_done = false;

thread::ThreadPool g_urltask_pool(6, 128); // TODO from conf
thread::ThreadPool g_responsetask_pool(6, 64); // TODO from conf
std::vector<std::string> g_seeds;
pthread_mutex_t target_mutex;

int init( );

int destroy();

void handleUrl(void * );

void handleResponse(void * );

#endif

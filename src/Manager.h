/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver at gmail.com
* @date 03/04/2014
* @description Crawler Manager header
*/

#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <vector>
#include <map>

#include "Worker.h"


void init ( );

void * assign (void * );

const std::string CONFIG = "crawl.config";
const std::string CONF_SEED = "seed";
const std::string CONF_DOWNLOAD_PATH = "download-path";
const std::string CONF_MAX_WORKER_COUNT = "max-worker-count";
const std::string CONF_MAX_URL_COUNT = "max-url-count";

const std::string DEFAULT_DOWNLOAD_PATH = "download";
const unsigned int DEFAULT_MAX_WORKER_COUNT = 1;
const unsigned int DEFAULT_MAX_URL_COUNT = 100;

std::map<std::string, std::string> conf_map;

std::vector<std::string> seed_vector;
std::string download_path;
unsigned int max_worker_count;
unsigned int worker_count;

#endif // MANAGER_H

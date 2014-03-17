/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver at gmail.com
* @date 03/04/2014
* @description Crawler Manager
*/

#include "Manager.h"

#include <pthread.h>

#include <fstream>


void init( )
{
    std::ifstream config(CONFIG);
    if (!config || !config.is_open()) {
        fprintf(stderr, "Can't open config config %s\n", CONFIG.c_str());
        return;
    }

    std::string line;
    size_t delim_pos;
    while (getline(config, line)) {
        if ((delim_pos = line.find('=')) == std::string::npos)
            continue;
        std::string prop(line.substr(0, delim_pos));
        std::string value(line.substr(delim_pos + 1));
        if (prop == CONF_SEED)
            seed_vector.push_back(value);
        else
            conf_map[prop] = value;
        fprintf(stderr, "%s=%s\n", prop.c_str(), value.c_str());
    }

    // set default if input has errors
    if ((Worker::download_path = conf_map[CONF_DOWNLOAD_PATH]) == "")
        Worker::download_path = DEFAULT_DOWNLOAD_PATH;

    if ((max_worker_count = atoi(conf_map[CONF_MAX_WORKER_COUNT].c_str())) == 0)
        max_worker_count = DEFAULT_MAX_WORKER_COUNT;

    if ((Worker::max_url_count = atoi(conf_map[CONF_MAX_URL_COUNT].c_str())) == 0) 
        Worker::max_url_count = DEFAULT_MAX_URL_COUNT;
}


void * assign (void * arg)
{
    std::string seed = *((std::string *)arg);
    Worker worker;
    worker.start(seed);
    return nullptr;
}


int main(int argc, char ** argv)
{
    init();
    freopen("debug.log", "w", stderr);

    pthread_t * tids = new pthread_t[max_worker_count]();

    for (int i = 0; i < seed_vector.size(); i++) {
        fprintf(stderr, "Seed %s\n", seed_vector[i].c_str());
        int ret = pthread_create(&tids[i], nullptr, assign, (void *)&seed_vector[i]);
        if (ret != 0)
            fprintf(stderr, "creating thread error with code %d\n", ret);
        else
            fprintf(stderr, "created thread %d\n", tids[i]);
    }

    for (int i = 0; i < seed_vector.size(); i++) {
        pthread_join(tids[i], nullptr);
    }
    
}

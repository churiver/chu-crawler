/**
* Copyright (c) 2013-2014
* @version 0.8.0
* @author Li Yu
* @email churiver at gmail.com
* @date 03/04/2014
* @description Crawler Manager header
*/

#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <queue>
#include <vector>

#include "Worker.h"

class Manager
{
public:


    void loadConfig (std::string & config_name );

    void allocateJob ( );

private:


//    std::queue<std:string> seed_url_pool;
    int max_worker_num;
    int max_link_per_worker;
//    std::vector<Worker> worker_pool;

};

#endif // MANAGER_H

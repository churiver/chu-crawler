/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/27/2014
* @description logger header
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>

#include <string>
#include <fstream>
#include <sstream>

#include "BlockingQueue.h"

namespace logs {


enum LogLevel
{
    DEBUG1, DEBUG2, DEBUG3, INFO, WARNING, ERROR
};

class Logger
{
public:
    Logger (LogLevel );

    ~Logger ( );

    static void init(std::string, std::string, int );

    static void destroy( );

    static LogLevel getMinlevel ( );

    template <typename T>
    Logger & operator<< (T const & value )
    {
        _oss << value;
        return *this;
    }

private:

    static void * daemon (void * );

    std::ostringstream _oss;
    
    static std::ofstream _ofs;
    static thread::BlockingQueue<std::string> _msg_que;
    // daemon controls 
    static bool _shutdown;
    static pthread_t _daemon_tid;
    // params
    static LogLevel _min_level;
    static int _low_watermark;
};


#define LOG(level) \
    if (logs::level < logs::Logger::getMinlevel()) \
        ; \
    else  \
        logs::Logger(logs::level) << "[" << __FUNCTION__ << "] "

};

#endif

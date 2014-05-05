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
    Logger (LogLevel, bool printToConsole = false );

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
    bool _printToConsole;
    LogLevel _level;
    
    static std::ofstream s_ofs;
    static thread::BlockingQueue<std::string> s_msg_queue;
    // daemon controls 
    static pthread_t s_daemon_tid;
    // params
    static LogLevel s_min_level;
    static int s_low_watermark;
};


#define LOG(level) \
    if (logs::level < logs::Logger::getMinlevel()) \
        ; \
    else  \
        logs::Logger(logs::level) << "[" << __FUNCTION__ << "] "

#define LOGnPRINT(level) \
    if (logs::level < logs::Logger::getMinlevel()) \
        ; \
    else  \
        logs::Logger(logs::level, true) << "[" << __FUNCTION__ << "] "
};

#endif

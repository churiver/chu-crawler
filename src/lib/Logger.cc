/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/27/2014
* @description logger
*/

#include "Logger.h"

#include <unistd.h>
#include <ctime>
#include <sys/syscall.h>
#include <unistd.h>

#include <iostream>
#include <map>

#define gettid() syscall(__NR_gettid)  

namespace logs {


const std::string LEVEL_STR[] = 
    {"DEBUG1","DEBUG2", "DEBUG3", "INFO", "WARNING", "ERROR"};
const std::string DEFAULT_LOG_FILE = "app.log";
const int DEFAULT_LOW_WATERMARK = 32;


// define static members in global scope
std::ofstream                       Logger::s_ofs;
thread::BlockingQueue<std::string>  Logger::s_msg_queue;
pthread_t                           Logger::s_daemon_tid;
LogLevel                            Logger::s_min_level;
int                                 Logger::s_low_watermark;


Logger::Logger (LogLevel level, bool printToConsole )
    : _level(level), _printToConsole(printToConsole)
{
    char datetime[16];
    time_t now = time(nullptr);
    strftime(datetime, sizeof(datetime), "%m/%d %T", localtime(&now));
    _oss << "[" << LEVEL_STR[level] << "]["  << datetime << "]" <<
    "[" << gettid() << "]";
}


Logger::~Logger ( )
{
    _oss << "\n";
    if (_printToConsole || (ERROR == _level)) {
        std::cerr << _oss.str();
    }
    s_msg_queue.put(_oss.str());
}


void Logger::init(std::string & logfile, const std::string & min_level, int low_watermark)
{
    // set log file
    if (logfile.empty() || ("" == logfile)) {
        logfile = DEFAULT_LOG_FILE;
    }
    s_ofs.open(logfile);
    if (!s_ofs.is_open()) {
        throw;
    }

    // set min_level
    if ("DEBUG1" == min_level) {
        s_min_level = DEBUG1;
    }
    else if ("DEBUG2" == min_level) {
        s_min_level = DEBUG2;
    }
    else if ("DEBUG3" == min_level) {
        s_min_level = DEBUG3;
    }
    else if ("WARNING" == min_level) {
        s_min_level = WARNING;
    }
    else if ("ERROR" == min_level) {
        s_min_level = ERROR;
    }
    else {
        s_min_level = INFO;
    }

    // set low watermark
    s_low_watermark = (0 == low_watermark) ? 
                        DEFAULT_LOW_WATERMARK : low_watermark;

    // run daemon thread
    int ret = pthread_create(&s_daemon_tid, nullptr, Logger::daemon, nullptr);
    if (ret != 0) {
        throw;
    }
}


void Logger::destroy ( )
{
    s_msg_queue.put("");
    //fprintf(stderr, "Logger destroying...\n");
    int ret = pthread_join(s_daemon_tid, nullptr);
    if (ret != 0) {
        std::cerr << "Logger destroy failed\n";
    }
    s_ofs.close();
    //std::cerr << "Logger destroyed\n";
}


LogLevel Logger::getMinlevel ( )
{
    return s_min_level;
}


void * Logger::daemon (void * arg)
{
    static int count = 0;
    while (true) {
        std::string log = s_msg_queue.take();
        if (log.empty()) {
            //std::cerr << "---- Logger daemon read empty string, will exit\n";
            return nullptr;
        }
        s_ofs << log;
        count++;
        if (count > s_low_watermark) {
            count = 0;
            s_ofs.flush();
        }
    }
    return nullptr;
}

/*
template <typename T>
Logger & Logger::operator<< (T const & value)
{
   s_oss << value;
   return *this;
}
*/
};

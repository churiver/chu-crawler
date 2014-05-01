/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/27/2014
* @description logger
*/

#include "Logger.h"

#include <unistd.h>

#include <map>

namespace logs {


const std::string LEVEL_STR[] = 
    {"DEBUG1","DEBUG2", "DEBUG3", "INFO", "WARNING", "ERROR"};
const std::string DEFAULT_LOG_FILE = "app.log";
const int DEFAULT_LOW_WATERMARK = 32;


// define static members in global scope
std::ofstream                       Logger::_ofs;
thread::BlockingQueue<std::string>  Logger::_msg_que;
bool                                Logger::_shutdown;
pthread_t                           Logger::_daemon_tid;
LogLevel                            Logger::_min_level;
int                                 Logger::_low_watermark;


Logger::Logger (LogLevel level )
{
    _oss << "[" << LEVEL_STR[level] << "]"; // TODO time/thread id
    if (ERROR == level) {
        destroy();
        sleep(1);
        throw; // TODO flush to file
    }
}


Logger::~Logger ( )
{
    _oss << "\n";
    _msg_que.put(_oss.str());
}


void Logger::init(std::string logfile, std::string min_level, int low_watermark)
{
    // set log file
    if (logfile.empty() || ("" == logfile)) {
        logfile = DEFAULT_LOG_FILE;
    }
    _ofs.open(logfile);
    if (!_ofs.is_open()) {
        throw;
    }

    // set min_level
    if ("DEBUG1" == min_level) {
        _min_level = DEBUG1;
    }
    else if ("DEBUG2" == min_level) {
        _min_level = DEBUG2;
    }
    else if ("DEBUG3" == min_level) {
        _min_level = DEBUG3;
    }
    else if ("WARNING" == min_level) {
        _min_level = WARNING;
    }
    else if ("ERROR" == min_level) {
        _min_level = ERROR;
    }
    else {
        _min_level = INFO;
    }

    // set low watermark
    _low_watermark = (0 == low_watermark) ? 
                        DEFAULT_LOW_WATERMARK : low_watermark;

    // run daemon thread
    int ret = pthread_create(&_daemon_tid, nullptr, Logger::daemon, nullptr);
    if (ret != 0) {
        throw;
    }
}


void Logger::destroy ( )
{
    _shutdown = true;
    _msg_que.put("");
    int ret = pthread_join(_daemon_tid, nullptr);
    if (ret != 0) {
        fprintf(stderr, "Logger destroy failed\n");
    }
}


LogLevel Logger::getMinlevel ( )
{
    return _min_level;
}


void * Logger::daemon (void * arg)
{
    int count = 0;
    while (_shutdown != true) {
        _ofs << _msg_que.take();
        count++;
        if (count > _low_watermark) {
            count = 0;
            _ofs.flush();
        }
    }
    _ofs.close();
    fprintf(stderr, "Logger shutting down\n");
    return nullptr;
}

/*
template <typename T>
Logger & Logger::operator<< (T const & value)
{
   _oss << value;
   return *this;
}
*/

};

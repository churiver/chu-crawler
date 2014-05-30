/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/15/2014
* @description task and thread pool header
*/

#ifndef THREAD_POOL_
#define THREAD_POOL_

#include "BlockingQueue.h"

//#include <vector>
#include <list>

namespace thread {


class Task
{
public:
    Task (void (* func_ptr)(void *), void * func_arg );
    
    ~Task ( );

    void operator() ( );

    void run ( );

private:
    void (* _func_ptr)(void *);
    void * _func_arg;
};


#define DEFAULT_THREAD_POOL_SIZE 5
#define DEFAULT_THREAD_PRIORITY 0
#define QUEUE_CAPACITY 1024*10

class ThreadPool
{
public:
    ThreadPool (size_t pool_size = DEFAULT_THREAD_POOL_SIZE,
                int priority = DEFAULT_THREAD_PRIORITY);

    ~ThreadPool ( );

    void increaseSizeTo (size_t );

    void setPriority (int );

    int destroy ( );

    void addTask (Task * );

private:
    void createThreads (size_t );

    static void * executeThread (void * );

    BlockingQueue<Task *> _task_queue;
    std::list<pthread_t> _thread_list;
    size_t _poolid;
    int _priority;
};

};
#endif

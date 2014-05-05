/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/15/2014
* @description task and thread pool
*/

#include "ThreadPool.h"

#include <cstdio>
#include <cstdlib>
#include <sys/syscall.h>
#include <unistd.h>

#define gettid() syscall(__NR_gettid)  

namespace thread {


Task::Task (void (* func_ptr)(void *), void * func_arg )
    : _func_ptr(func_ptr), _func_arg(func_arg)
{ }


Task::~Task ( )
{ }


void Task::operator() ( )
{
    (* _func_ptr)(_func_arg);
}


void Task::run ( )
{
    (* _func_ptr)(_func_arg);
}


ThreadPool::ThreadPool (size_t pool_size, size_t que_capacity )
    : _task_queue(que_capacity)
{
    static size_t s_next_poolid = 0;
    _poolid = s_next_poolid++;

    int thread_count = 0;
    pthread_t tid;

    for ( int i = 0; i < pool_size; i++) {
        int ret = pthread_create(&tid, nullptr, ThreadPool::executeThread, (void *)this );
        if (ret != 0) {
            fprintf(stderr, "pool creating pthread failed\n");
            continue;
        }
        thread_count++;
        _thread_list.push_back(tid);
    }

//    fprintf(stderr, "pool %d. created %d pthreads\n", _poolid, thread_count);
    if (0 == thread_count)
        throw;
}


ThreadPool::~ThreadPool ( ) 
{ }


int ThreadPool::destroy ( )
{
//    fprintf(stderr, "Pool %d destroying...\n", _poolid); // DEBUG
    _task_queue.clear();
    _task_queue.interrupt();

    std::vector<pthread_t>::iterator it = _thread_list.begin();
    for (it; it != _thread_list.end(); it++) {
        int ret = pthread_join(*it, nullptr);
        if (ret != 0) {
            fprintf(stderr, "pool %d. pthread %d returned %d\n", _poolid, *it, ret);
        }
    }
}


bool ThreadPool::addTask (Task * task )
{
    _task_queue.put(task);
}


void * ThreadPool::executeThread (void * arg )
{
    ThreadPool * self = (ThreadPool *)arg;
    while (true) {
        // block at take() if queue is empty
        Task * task = self->_task_queue.take();
        task->run(); // (* task)();

        delete task;
    }

    return nullptr;
}

/*
void ThreadPool::stopThread (void * arg )
{
    fprintf(stderr, "thread %d quit\n", gettid());    
}
*/
};

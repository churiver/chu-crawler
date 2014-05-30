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
#include <sys/time.h>
#include <sys/resource.h>

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


ThreadPool::ThreadPool (size_t pool_size, int priority )
    : _task_queue(QUEUE_CAPACITY), _priority(priority)
{
    static size_t s_next_poolid = 0;
    _poolid = s_next_poolid++;

    createThreads(pool_size);
/*
    int thread_count = 0;
    pthread_t tid;

    for (int i = 0; i < pool_size; i++) {
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
        throw;*/
}


ThreadPool::~ThreadPool ( ) 
{ }


void ThreadPool::increaseSizeTo (size_t new_size )
{
    size_t current_size = _thread_list.size();

    if (current_size < new_size) {
        createThreads(new_size - current_size);
    }
}


void ThreadPool::setPriority (int priority )
{
    _priority = priority;
}


int ThreadPool::destroy ( )
{
    _task_queue.clear();
    _task_queue.interrupt();

    std::list<pthread_t>::iterator it = _thread_list.begin();
    for (it; it != _thread_list.end(); it++) {
        int ret = pthread_join(*it, nullptr);
        if (ret != 0) {
            fprintf(stderr, "pool %d. pthread %d returned %d\n", _poolid, *it, ret);
        }
    }
}


void ThreadPool::addTask (Task * task )
{
    _task_queue.put(task);
}


void ThreadPool::createThreads (size_t thread_num )
{
    int thread_count = 0;
    pthread_t tid;

    for (int i = 0; i < thread_num; i++) {
        int ret = pthread_create(&tid, nullptr, ThreadPool::executeThread, (void *)this );
        if (ret != 0) {
            fprintf(stderr, "pool creating pthread failed\n");
            continue;
        }
        thread_count++;
        _thread_list.push_back(tid);
    }

//    fprintf(stderr, "pool %d. created %d pthreads\n", _poolid, thread_count);
    if (0 == thread_count) {
        throw;
    }
}


void * ThreadPool::executeThread (void * arg )
{
    ThreadPool * self = (ThreadPool *)arg;
    
    if (self->_priority != 0) {
        int ret = setpriority(PRIO_PROCESS, gettid(), self->_priority);
    }

    while (true) {
        // will block at take() if queue is empty
        Task * task = self->_task_queue.take();
        task->run(); // (* task)();

        delete task;
    }

    return nullptr;
}

};

#include "blocking_queue.h"

#include <cstdio>

template <class T>
blocking_queue<T>::blocking_queue (size_t capacity )
{
    _capacity = capacity;

    pthread_mutex_init(&mutex_queue, NULL);
    pthread_cond_init(&cond_is_empty, NULL);
    pthread_cond_init(&cond_is_full, NULL);
}

template <class T>
blocking_queue<T>::~blocking_queue ( )
{
    pthread_mutex_destroy(&mutex_queue);
    pthread_cond_destroy(&cond_is_empty);
    pthread_cond_destroy(&cond_is_full);
}

template <class T>
void blocking_queue<T>::put (const T & val )
{
    pthread_mutex_lock(&mutex_queue);
    
    fprintf(stderr, "put: 1. wait\n");
    while (container.size() == _capacity) {
        int rc = pthread_cond_wait(&cond_is_full, &mutex_queue);
        fprintf(stderr, "put: 2. cond_wait return %d\n", rc);
    }

    fprintf(stderr, "put: 3. waken up from wait\n");
    container.push_back(val);
    
    fprintf(stderr, "put: 4. signal cond_is_empty. contanier size %d\n", container.size());
    int rc = pthread_cond_signal(&cond_is_empty);
    fprintf(stderr, "put: 5. cond_signal return %d\n", rc);

    pthread_mutex_unlock(&mutex_queue);
}

template <class T>
T blocking_queue<T>::take ( )
{
    pthread_mutex_lock(&mutex_queue);

    fprintf(stderr, "take: 1. wait. container size %d\n", container.size());
    while (container.size() == 0) {
        int rc = pthread_cond_wait(&cond_is_empty, &mutex_queue);
        fprintf(stderr, "take: 2. cond_wait return %d\n", rc);
    }

    fprintf(stderr, "take: 3. waken up from wait\n");
    T val = container.front();
    container.pop_front();

    fprintf(stderr, "take: 4. signal cond_is_full\n");
    int rc = pthread_cond_signal(&cond_is_full);
    fprintf(stderr, "take: 5. cond_signal return %d\n", rc);

    pthread_mutex_unlock(&mutex_queue);
    return val;
}

template <class T>
size_t blocking_queue<T>::size ( )
{
    return container.size();
}

template <class T>
bool blocking_queue<T>::empty ( )
{
    return container.empty();
}

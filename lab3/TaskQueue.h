#pragma once


#include "sthread.h"
#include <queue>

typedef void (*handler_t) (void *); 

struct Task {
    handler_t handler;
    void* arg;
};

/*
 * ------------------------------------------------------------------
 * TaskQueue --
 * 
 *      A thread-safe task queue. This queue should be implemented
 *      as a monitor.
 *
 * ------------------------------------------------------------------
 */
class TaskQueue {
    private:
    std::queue<Task> q;
    smutex_t mutex;
    scond_t queue_not_empty;

    public:
    TaskQueue();
    ~TaskQueue();

    void enqueue(Task task);
    Task dequeue();

    int size();
    bool empty();
};



#include "TaskQueue.h"

TaskQueue::
TaskQueue()
{
    smutex_init(&mutex);
    scond_init(&queue_not_empty);
}

TaskQueue::
~TaskQueue()
{
    smutex_destroy(&mutex);
    scond_destroy(&queue_not_empty);
}

/*
 * ------------------------------------------------------------------
 * size --
 *
 *      Return the current size of the queue.
 *
 * Results:
 *      The size of the queue.
 *
 * ------------------------------------------------------------------
 */
int TaskQueue::
size()
{
    smutex_lock(&mutex);
    int size = q.size();
    smutex_unlock(&mutex);
    return size;
}

/*
 * ------------------------------------------------------------------
 * empty --
 *
 *      Return whether or not the queue is empty.
 *
 * Results:
 *      The true if the queue is empty and false otherwise.
 *
 * ------------------------------------------------------------------
 */
bool TaskQueue::
empty()
{
    smutex_lock(&mutex);
    bool empty = q.empty();
    smutex_unlock(&mutex);
    return empty;
}

/*
 * ------------------------------------------------------------------
 * enqueue --
 *
 *      Insert the task at the back of the queue.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void TaskQueue::
enqueue(Task task)
{
    smutex_lock(&mutex);
    q.push(task);
    scond_signal(&queue_not_empty, &mutex);
    smutex_unlock(&mutex);
}

/*
 * ------------------------------------------------------------------
 * dequeue --
 *
 *      Remove the Task at the front of the queue and return it.
 *      If the queue is empty, block until a Task is inserted.
 *
 * Results:
 *      The Task at the front of the queue.
 *
 * ------------------------------------------------------------------
 */
Task TaskQueue::
dequeue()
{
    smutex_lock(&mutex);
    while(empty()){
        scond_wait(&queue_not_empty, &mutex);
    }
    Task t = q.front();
    q.pop();

    smutex_unlock(&mutex);
    return t;
}



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
    return size_q;
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
    return size_q == 0; 
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
    size_q ++;
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
    size_q--;

    smutex_unlock(&mutex);
    return t;
}


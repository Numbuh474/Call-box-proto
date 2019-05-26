#include "queue.h"

unsigned int queue_wrap(struct Queue * queue, int offset)
{
    unsigned int result = (queue->head + offset + queue->capacity) % queue->capacity;
    return result;
}
int queue_create(struct Queue* queue, queue_t * array, int capacity)
{
    if (queue && array && capacity > 0)
    {
        queue->array = array;
        queue->capacity = capacity;
        queue->length = 0;
        queue->head = 0;
        return 1;
    }
    else return 0;
}

int queue_enqueue(struct Queue * queue, queue_t value)
{
    if(queue->length < queue->capacity)
    {
        unsigned int tail = queue_wrap(queue, queue->length);
        queue->array[tail] = value;
        queue->length++;
        return 1;
    }
    else return 0;
}

queue_t queue_dequeue(struct Queue * queue)
{
    if (queue->length > 0)
    {
        unsigned int result = queue->array[queue->head];
        queue->head = queue_wrap(queue, 1);
        queue->length--;
        return result;
    }
    else return 0;
}

queue_t queue_get(struct Queue * queue, unsigned int index)
{
    if (index < queue->length)
    {
        return queue->array[queue_wrap(queue, index)];
    }
    else
    {
        return 0;
    }
}

int queue_set(struct Queue * queue, unsigned int index, queue_t data)
{
    if (index < queue->length)
    {
        queue->array[queue_wrap(queue, index)] = data;
        return 1;
    }
    else return 0;
}

int queue_insert(struct Queue * queue, unsigned int index, queue_t data)
{
    if (index <= queue->length && queue->length < queue->capacity)
    {
        queue_t tmp;
        queue->length++;

        for ( ; index < queue->length; index++)
        {
            tmp = queue_get(queue, index);
            queue_set(queue, index, data);
            data = tmp;
        }
        return 1;
    }
    else return 0;
}

queue_t remove(struct Queue* queue, int index)
{
    if (index < queue->length && index >= 0)
    {
        queue->length--;
        queue_t result = queue_get(queue, index);
        for ( ; index < queue->length; index++)
        {
            queue_set(queue, index, queue_get(queue, index+1));
        }
        return result;
    }
    else return 0;
}

int queue_index_of(struct Queue * queue, queue_t search)
{
    unsigned int i;
    for (i = 0; i < queue->length; i++)
    {
        if (queue_get(queue, i)==search)
            return i;
    }
    return -1;
}

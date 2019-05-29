#ifndef QUEUE_H_
#define QUEUE_H_

#ifndef QUEUE_T
#define QUEUE_T
typedef unsigned int queue_t;
#endif

struct Queue
{
    queue_t * array;
    unsigned int capacity;
    unsigned int length;
    unsigned int head;
};

unsigned int queue_wrap(struct Queue * queue, int offset);
int queue_create(struct Queue * queue, queue_t * array, int capacity);
int queue_enqueue(struct Queue * queue, queue_t value);
queue_t queue_dequeue(struct Queue * queue);
queue_t queue_get(struct Queue * queue, unsigned int index);
int queue_set(struct Queue * queue, unsigned int index, queue_t data);
int queue_insert(struct Queue * queue, unsigned int index, queue_t data);
queue_t queue_remove(struct Queue * queue, unsigned int index);
int queue_index_of(struct Queue * queue, queue_t search);

#endif /* QUEUE_H_ */

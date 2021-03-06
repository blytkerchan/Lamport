#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>

#define T int
#define SIZE 64

struct LamportQueue
{
    atomic_size_t front_;
    atomic_size_t back_;
    size_t cached_front_;
    size_t cached_back_;
    T data_[SIZE];
};

void LamportQueue_init(struct LamportQueue *queue)
{
       atomic_init(&queue->front_, 0);
       atomic_init(&queue->back_, 0);
       queue->cached_front_ = queue->cached_back_ = 0;
}

bool LamportQueue_push(struct LamportQueue *queue, T elem)
{
    size_t b, f;
    b = atomic_load_explicit(&queue->back_, memory_order_relaxed);
    f = queue->cached_front_;
    if ((b + 1) % SIZE == f)
    {
	    queue->cached_front_ = f = atomic_load_explicit(&queue->front_, memory_order_acquire);
    }
    else
    { /* front can only increase since the last time we read it, which means we can only get more space to push into.
	 If we still have space left from the last time we read, we don't have to read again. */ }
    if ((b + 1) % SIZE == f)
    {
        return false;
    }
    else
    { /* not full */ }
    queue->data_[b] = elem;
    atomic_store_explicit(&queue->back_, (b + 1) % SIZE, memory_order_release);
    return true;
}

bool LamportQueue_pop(struct LamportQueue *queue, T *elem)
{
    size_t b, f;
    f = atomic_load_explicit(&queue->front_, memory_order_relaxed);
    b = queue->cached_back_;
    if (b == f)
    {
	    queue->cached_back_ = b = atomic_load_explicit(&queue->back_, memory_order_acquire);
    }
    else
    { /* back can only increase since the last time we read it, which means we can only get more items to pop from.
	 If we still have items left from the last time we read, we don't have to read again. */ }
    if (b == f)
    {
        return false;
    }
    else
    { /* not empty */ }
    *elem = queue->data_[f];
    atomic_store_explicit(&queue->front_, (f + 1) % SIZE, memory_order_release);
    return true;
}

void* producer(void *p)
{
    struct LamportQueue *queue = (struct LamportQueue*)p;
    int i;
    for (i = 0; i < 100;)
    {
        if(LamportQueue_push(queue, 36)) ++i;
    }

    return 0;
}
void* consumer(void *p)
{
    struct LamportQueue *queue = (struct LamportQueue*)p;
    int i;
    for (i = 0; i < 100;)
    {
        int v;
        if(LamportQueue_pop(queue, &v))
	{
            ++i;
	    assert(v == 36);
        }
    }

    return 0;
}

int main()
{
    struct LamportQueue queue;
    LamportQueue_init(&queue);
    pthread_t t[2];
    pthread_create(&t[0], 0, producer, &queue);
    pthread_create(&t[1], 0, consumer, &queue);
    pthread_join(t[1], 0);
    pthread_join(t[0], 0);
}



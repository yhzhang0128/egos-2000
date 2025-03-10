/* Implementing a queue data structure helps you implement cooperative threads,
 * but this is optional and you can maintain arrays instead of queues.
 */

/*
 * queue_t is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how queues are
 * represented.  They see and manipulate only queue_t's.
 */
typedef struct queue *queue_t;

/*
 * Return an empty queue.  Returns NULL on error (out of memory).
 */
queue_t queue_new();

/*
 * Enqueue an item.
 * Return 0 (success) or -1 (failure).  (A reason for failure may be
 * that there is not enough memory left.)
 */
int queue_enqueue(queue_t queue, void* item);

/*
 * Dequeue and return the first item from the queue in *pitem, but only if
 * pitem != NULL.  If pitem == NULL, then just dequeue.
 * Return 0 (success) if queue is nonempty, or -1 (failure)
 * if queue is empty.
 */
int queue_dequeue(queue_t queue, void** pitem);

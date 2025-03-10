/* Implementing a queue data structure helps you implement cooperative threads,
 * but this is optional and you can maintain arrays instead of queues.
 */

#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

struct queue {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
};
typedef struct queue * queue_t;

queue_t queue_new() {
    queue_t queue      = malloc(sizeof(*queue));
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
    return queue;
}

int queue_enqueue(queue_t queue, void* item) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
    return 0;
}

int queue_dequeue(queue_t queue, void** pitem) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
    return 0;
}

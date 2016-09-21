#ifndef DART__MPI__DART_ACTIVE_MESSAGES_H_
#define DART__MPI__DART_ACTIVE_MESSAGES_H_


#include <dash/dart/if/dart_types.h>

struct dart_amsgq;

typedef struct dart_amsgq* dart_amsgq_t;

typedef void (rfunc_t) (void *);

typedef struct dart_amsg {
  void       *data;       // the data to be passed to to the handler specified during queue setup
  int         data_size;  // the size of the data to passed to the handler
} dart_amsg_t;

/**
 * Initialize an active message queue of size \c size on all units in team.
 *
 * This is a collective operation involving all units in team.
 */
dart_amsgq_t
dart_amsg_openq(int size, dart_team_t team, rfunc_t *handler);

/**
 * Try to send an active message to unit \c target through message queue \c amsgq.
 * At the target, a task will be created that executes \c fn with arguments \c data.
 * The argument data of size \c data_size will be copied to the target unit's message queue.
 * The call fails if there is not sufficient space available in the target's message queue.
 *
 * Implementation: The queue is shortly locked through a compare_and_swap atomic to determine
 *                 the position in the queue and update the queue tail using fetch_and_op.
 *                 The queue is then released before the actual message is copied.
 *
 * Note: Although the built-in notification of dart_locks, We do not use them since:
 *       1) One global lock would potentially harm performance (no two message queues could be used at the same time)
 *       2) We would need N locks for fine-grained locking (leading to O(N) required memory per unit and O(N^2) global memory)
 *
 * Note: All data required to execute the function contained in the active message must be contained in the data argument,
 *       i.e., no external references can be handled at the moment.
 */
dart_ret_t
dart_amsg_trysend(dart_unit_t target, dart_amsgq_t amsgq, const void *data, size_t data_size);

/**
 * If available, dequeue all messages in the local queue by calling the function and on the supplied data argument (see dart_amsg_t).
 *
 * Implementation: The local queue will be locked (again using compare_and_swap) to grab a copy of the current content
 *                 and released before processing starts.
 */
dart_ret_t
dart_amsg_process(dart_amsgq_t amsgq);

/**
 * Close the queue, discarding all remaining messages and deallocating all allocated memory.
 */
dart_ret_t
dart_amsg_closeq(dart_amsgq_t amsgq);


#endif /* DART__MPI__DART_ACTIVE_MESSAGES_H_ */

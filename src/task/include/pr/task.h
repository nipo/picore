/**
 * @file task.h
 * @defgroup pr_task Task Scheduler
 * @brief Cooperative task scheduling system for non-preemptive multitasking
 *
 * The task library provides a cooperative scheduling model with round-robin task
 * execution. Tasks are scheduled in a queue and serviced sequentially, with each
 * task running to completion before the next is serviced.
 *
 * Key features:
 * - Non-preemptive cooperative scheduling
 * - Immediate or delayed task execution
 * - Thread-safe task queue operations using spin locks
 * - Integration with Pico SDK alarm system for timeouts
 * - Composable task structures for embedding in parent contexts
 *
 * Typical usage pattern:
 * @code
 * struct pr_task_queue queue;
 * struct pr_task my_task;
 *
 * void my_handler(struct pr_task *task) {
 *     // Do work...
 *     // Optionally reschedule: pr_task_exec(task);
 * }
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *     pr_task_init(&my_task, &queue, my_handler);
 *     pr_task_exec(&my_task);
 *
 *     for (;;) {
 *         pr_task_queue_run_until_empty(&queue);
 *     }
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_TASK_H_
#define PR_TASK_H_

#include <stdint.h>
#include <hardware/sync.h>
#include <pico/time.h>
#include <pr/struct_compose.h>

struct pr_task;
struct pr_task_queue;

/**
 * @brief Task handler function type
 *
 * Handler functions are called when a task is executed from the queue.
 * The task runs to completion; there is no preemption.
 *
 * @param task Pointer to the task being executed
 */
typedef void pr_task_handler_t(struct pr_task *);

/**
 * @brief Helper macro for composing tasks within parent structures
 *
 * This macro generates a function to retrieve the parent structure pointer
 * from an embedded task member. Use this when embedding pr_task structures
 * within larger application contexts.
 *
 * Example:
 * @code
 * struct app_context {
 *     struct pr_task my_task;
 *     int data;
 * };
 *
 * PR_TASK_STRUCT_COMPOSE(app_context, my_task);
 *
 * void handler(struct pr_task *task) {
 *     struct app_context *ctx = app_context_from_my_task(task);
 *     // Access ctx->data...
 * }
 * @endcode
 *
 * @param struct_name Name of the parent structure type
 * @param field Name of the pr_task field within the parent structure
 */
#define PR_TASK_STRUCT_COMPOSE(struct_name, field)                        \
    PR_STRUCT_COMPOSE(struct_name, pr_task, field)

/**
 * @brief Flag indicating task is pending in the queue
 */
#define PR_TASK_FLAG_PENDING 1
#define PR_TASK_FLAG_WAITING 2

/**
 * @brief Task structure
 *
 * Represents a schedulable unit of work. Tasks are organized in a
 * linked list within a task queue and are executed by their handler
 * function. All fields are internal.
 */
struct pr_task
{
    struct pr_task *next;           /**< Next task in the queue */
    uint32_t flags;                 /**< Task flags (PR_TASK_FLAG_*) */
    struct pr_task_queue *queue;    /**< Queue this task belongs to */
    pr_task_handler_t *handler;     /**< Handler function to execute */
    absolute_time_t exec_at;        /**< Future execution time for waiting tasks */
};

/**
 * @brief Task queue structure
 *
 * Manages a queue of pending tasks. Tasks are serviced in round-robin order.
 * The queue uses spin locks for thread-safe operation.
 */
struct pr_task_queue
{
    struct pr_task *first;    /**< First task in the queue */
    struct pr_task *last;     /**< Last task in the queue */
    struct pr_task *waiting;  /**< Ordered timed waiting queue */
    spin_lock_t *lock;        /**< Spin lock for thread-safe operations */
    int lock_no;              /**< Spin lock number */
    alarm_id_t alarm;         /**< Alarm for timed execution */
};

/**
 * @brief Initialize a task
 *
 * Prepares a task structure for use. The task is not scheduled; call
 * pr_task_exec() to schedule it for execution.
 *
 * @param task Pointer to the task structure to initialize
 * @param queue Pointer to the task queue this task belongs to
 * @param handler Handler function to call when task executes
 */
void pr_task_init(struct pr_task *task,
                     struct pr_task_queue *queue,
                     pr_task_handler_t *handler);

/**
 * @brief Schedule a task for immediate execution
 *
 * Adds the task to its queue for execution as soon as possible. If the task
 * is already pending, this call has no effect. Thread-safe.
 *
 * @param t Pointer to the task to execute
 * @return 0 on success, -1 on error (null task or queue)
 */
int pr_task_exec(struct pr_task *t);

/**
 * @brief Schedule a task for delayed execution (microseconds)
 *
 * Schedules the task to execute after the specified delay in microseconds.
 * Uses the Pico SDK alarm system. If the task already has a pending alarm,
 * it is cancelled and replaced with the new delay.
 *
 * @param t Pointer to the task to execute
 * @param us Delay in microseconds before execution
 *
 * @note The task will be scheduled after the delay expires, but actual
 *       execution time depends on other pending tasks in the queue.
 */
void pr_task_exec_in_us(struct pr_task *t, uint64_t us);

/**
 * @brief Schedule a task for delayed execution (milliseconds)
 *
 * Schedules the task to execute after the specified delay in milliseconds.
 * Uses the Pico SDK alarm system. If the task already has a pending alarm,
 * it is cancelled and replaced with the new delay.
 *
 * @param t Pointer to the task to execute
 * @param ms Delay in milliseconds before execution
 *
 * @note The task will be scheduled after the delay expires, but actual
 *       execution time depends on other pending tasks in the queue.
 */
void pr_task_exec_in_ms(struct pr_task *t, uint64_t ms);

/**
 * @brief Cancel a pending task
 *
 * Cancels any pending delayed execution alarm for this task.
 *
 * @param t Pointer to the task to cancel
 * @return Status code
 *
 * @note This only cancels the alarm; if the task is already in the queue,
 *       it will still execute.
 */
int pr_task_cancel(struct pr_task *t);

/**
 * @brief Initialize a task queue
 *
 * Prepares a task queue for use. This must be called before any tasks
 * can be scheduled on the queue. Initializes the default alarm pool
 * and claims a spin lock for thread safety.
 *
 * @param q Pointer to the task queue structure to initialize
 */
void pr_task_queue_init(struct pr_task_queue *q);

/**
 * @brief Check if task queue is empty
 *
 * Thread-safe check for whether the queue has any pending tasks.
 *
 * @param q Pointer to the task queue
 * @return true if queue is empty, false if tasks are pending
 */
bool pr_task_queue_is_empty(struct pr_task_queue *q);

/**
 * @brief Execute all pending tasks in the queue
 *
 * Processes all tasks currently in the queue, running each task's handler
 * to completion. Returns when the queue becomes empty. This is typically
 * called in the main application loop.
 *
 * @param q Pointer to the task queue
 *
 * @note Tasks may schedule themselves or other tasks during execution.
 *       Only tasks that were pending at the start of this call or added
 *       by executed tasks will run; this function returns when the queue
 *       is empty at the time of checking.
 */
void pr_task_queue_run_until_empty(struct pr_task_queue *q);

/** @} */ // end of pr_task group

#endif

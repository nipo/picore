/**
 * @file struct_compose.h
 * @defgroup pr_struct_compose Structure Composition
 * @brief Macro for container-of pattern implementation
 *
 * This module provides the PR_STRUCT_COMPOSE macro which implements the
 * "container-of" pattern used extensively throughout Pico-RE. It generates
 * helper functions to retrieve a parent structure pointer from an embedded
 * member pointer.
 *
 * This pattern is fundamental to Pico-RE's design, allowing structures like
 * pr_task to be embedded within larger application contexts while maintaining
 * type safety and providing convenient accessor functions.
 *
 * Example usage:
 * @code
 * struct app_context {
 *     struct pr_task task;
 *     struct pr_fifo fifo;
 *     int state;
 * };
 *
 * // Generate accessor functions
 * PR_STRUCT_COMPOSE(app_context, pr_task, task);
 * PR_STRUCT_COMPOSE(app_context, pr_fifo, fifo);
 *
 * void task_handler(struct pr_task *task) {
 *     // Retrieve parent structure from embedded task pointer
 *     struct app_context *app = app_context_from_task(task);
 *     // Access app->state, app->fifo, etc.
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_STRUCT_COMPOSE_H_
#define PR_STRUCT_COMPOSE_H_

#include <stdint.h>

/**
 * @brief Generate container-of accessor functions
 *
 * This macro generates two inline functions that convert a pointer to an
 * embedded structure member back to a pointer to the containing structure.
 * Both mutable and const variants are generated.
 *
 * Generated functions:
 * - `struct surrounding *surrounding##_from_##field(struct included *element_ptr)`
 * - `const struct surrounding *const_##surrounding##_from_##field(const struct included *element_ptr)`
 *
 * @param surrounding Name of the parent/containing structure type
 * @param included Name of the embedded structure type
 * @param field Name of the field within the surrounding structure
 *
 * @note This macro uses offsetof() to calculate the offset of the field,
 *       ensuring safe pointer arithmetic that works with any alignment.
 *
 * Example:
 * @code
 * struct outer {
 *     int x;
 *     struct inner y;
 * };
 *
 * PR_STRUCT_COMPOSE(outer, inner, y);
 *
 * // Now you can use:
 * struct inner *inner_ptr = &my_outer.y;
 * struct outer *outer_ptr = outer_from_y(inner_ptr);
 * @endcode
 */
#define PR_STRUCT_COMPOSE(surrounding, included, field)                 \
                                                                        \
    static inline struct surrounding *                                  \
    surrounding##_from_##field(struct included *element_ptr)            \
    {                                                                   \
        const ptrdiff_t field_offset = offsetof(struct surrounding, field); \
        return (struct surrounding *)((uintptr_t)element_ptr - field_offset); \
    }                                                                   \
                                                                        \
    static inline const struct surrounding *                            \
    const_##surrounding##_from_##field(const struct included *element_ptr) \
    {                                                                   \
        const ptrdiff_t field_offset = offsetof(struct surrounding, field); \
        return (const struct surrounding *)((uintptr_t)element_ptr - field_offset); \
    }

/** @} */ // end of pr_struct_compose group

#endif

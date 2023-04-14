#ifndef PR_STRUCT_COMPOSE_H_
#define PR_STRUCT_COMPOSE_H_

#include <stdint.h>

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

#endif

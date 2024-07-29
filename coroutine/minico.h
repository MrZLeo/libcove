#ifndef LIBCOVE_COROUTINE_H
#define LIBCOVE_COROUTINE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum co_state {
    CO_STATE_INIT,
    CO_STATE_RUN,
    CO_STATE_YIELD,
    CO_STATE_FINISH
} co_state_t;

struct co_block;

typedef int (*co_task_t)(struct co_block *co, void *data);

struct co_block {
    co_state_t state;
    co_task_t fn;
    void *data;
    uint32_t label;
};

[[gnu::always_inline]]
static inline void co_init(struct co_block *co, co_task_t fn, void *data) {
    co->state = CO_STATE_INIT;
    co->fn = fn;
    co->data = data;
    co->label = 0;
}

[[gnu::always_inline]]
static inline int co_poll(struct co_block *co) {
    if (co->state == CO_STATE_FINISH)
        return 0;

    co->state = CO_STATE_RUN;
    if (co->fn) {
        return co->fn(co, co->data);
    }
    co->state = CO_STATE_FINISH;
    return 0;
}

[[gnu::always_inline]]
static inline int co_stop(struct co_block *co) {
    co->state = CO_STATE_FINISH;
    return 0;
}

[[gnu::always_inline]]
static inline bool co_done(struct co_block *co) {
    return co->state == CO_STATE_FINISH;
}

#define CO_BEGIN(co)       \
    switch ((co)->label) { \
    case 0:

#define CO_YIELD(co, value)           \
    do {                              \
        (co)->label = __LINE__;       \
        (co)->state = CO_STATE_YIELD; \
        return (value);               \
    case __LINE__:                    \
        break;                        \
    } while (0)

#define CO_END(co)                 \
    (co)->state = CO_STATE_FINISH; \
    }

#endif  // LIBCOVE_COROUTINE_H

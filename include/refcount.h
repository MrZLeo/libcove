#ifndef LIBCOVE_RC_H
#define LIBCOVE_RC_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

struct rc {
    uint32_t ref;
};

struct arc {
    atomic_uint ref;
};

// ref count for single thread
void rc_init(struct rc *refcount);
void rc_inc(struct rc *refcount);
bool rc_dec(struct rc *refcount);
bool rc_cmp(const struct rc *refcount, const uint32_t val);

// atomic ref count for multi-thread
void arc_init(struct arc *refcount);
void arc_inc(struct arc *refcount);
bool arc_dec(struct arc *refcount);
bool arc_cmp(const struct arc *refcount, const uint32_t val);

#endif  // LIBCOVE_RC_H



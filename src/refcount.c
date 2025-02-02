#include "refcount.h"
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>

// ref count for single thread
inline void rc_init(struct rc *refcount) {
    if (!refcount)
        return;
    refcount->ref = 0;
}

inline void rc_inc(struct rc *refcount) {
    if (!refcount)
        return;
    __auto_type rc = refcount->ref;
    if (rc == UINT32_MAX)
        abort();
    rc++;
    refcount->ref = rc;
}

// return whether rc reach 0, i.e., no ref to specific object
inline bool rc_dec(struct rc *refcount) {
    if (!refcount)
        return false;
    __auto_type old_val = refcount->ref--;
    return old_val == 1 ? true : false;
}

inline bool rc_cmp(const struct rc *refcount, const uint32_t val) {
    if (!refcount)
        return false;
    const __auto_type rc = refcount->ref;
    return rc == val;
}

// atomic ref count for multi-thread

// init operation is multi-entry
inline void arc_init(struct arc *refcount) {
    if (!refcount)
        return;
    refcount->ref = 0;
}

inline void arc_inc(struct arc *refcount) {
    if (!refcount)
        return;
    const __auto_type old_val =
        atomic_fetch_add_explicit(&(refcount->ref), 1, memory_order_acq_rel);
    if (old_val == INT32_MAX)
        abort();
}

inline bool arc_dec(struct arc *refcount) {
    if (!refcount)
        return false;
    const __auto_type old_val =
        atomic_fetch_sub_explicit(&(refcount->ref), 1, memory_order_acq_rel);
    return old_val == 1 ? true : false;
}

inline bool arc_cmp(const struct arc *refcount, const uint32_t val) {
    if (!refcount)
        return false;
    return atomic_load_explicit(&refcount->ref, memory_order_acquire) == val;
}

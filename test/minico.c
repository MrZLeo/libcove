#include "minico.h"
#include <criterion/criterion.h>

// A simple coroutine function for testing.
static int simple_co_fn(struct co_block *co, [[maybe_unused]] void *data) {
    CO_BEGIN(co);
    CO_YIELD(co, 1);
    CO_END(co);
    return 0;
}

// Test initialization of the coroutine.
Test(coroutine, initialization) {
    struct co_block co;
    co_init(&co, simple_co_fn, NULL);
    cr_assert_not_null(
        co.fn,
        "Coroutine function should not be NULL after initialization."
    );
}

// Test co_poll on an uninitialized coroutine.
Test(coroutine, poll_uninitialized) {
    struct co_block co = { 0 };
    int result = co_poll(&co);
    cr_assert_eq(
        result,
        0,
        "Polling an uninitialized coroutine should return 0."
    );
}

// Test co_stop on an uninitialized coroutine.
Test(coroutine, stop_uninitialized) {
    struct co_block co = { 0 };
    co_stop(&co);  // Should not cause any error or undefined behavior.
    cr_assert(true, "Stopping an uninitialized coroutine should be safe.");
}

// Test co_done on an uninitialized coroutine.
Test(coroutine, done_uninitialized) {
    struct co_block co = { 0 };  // Simulate uninitialized coroutine.
    bool done = co_done(&co);
    cr_assert_not(
        done,
        "An uninitialized coroutine should not be marked as done."
    );
}

// Test co_poll on a finished coroutine.
Test(coroutine, poll_finished) {
    struct co_block co;
    co_init(&co, simple_co_fn, NULL);
    co_poll(&co);               // Move to the end of the coroutine.
    int result = co_poll(&co);  // Poll again after finishing.
    cr_assert_eq(result, 0, "Polling a finished coroutine should return 0.");
}

// A coroutine function that yields multiple times before finishing.
static int test_co_fn(struct co_block *co, [[maybe_unused]] void *data) {
    CO_BEGIN(co);
    CO_YIELD(co, 1);  // First yield point, should return 1.
    CO_YIELD(co, 2);  // Second yield point, should return 2.
    CO_END(co);       // Finish the coroutine.
    return 0;         // Return value after finishing.
}

Test(coroutine, complex_test) {
    struct co_block co;
    co_init(
        &co,
        test_co_fn,
        NULL
    );  // Initialize the coroutine with the test function.

    // First poll: should execute up to the first yield point.
    int result = co_poll(&co);
    cr_assert_eq(result, 1, "First poll did not yield expected result.");

    // Second poll: should execute up to the second yield point.
    result = co_poll(&co);
    cr_assert_eq(result, 2, "Second poll did not yield expected result.");

    // Stop the coroutine explicitly.
    co_stop(&co);
    cr_assert(
        co_done(&co),
        "Coroutine should be marked as done after stopping."
    );

    // Attempt to poll the coroutine again after stopping.
    result = co_poll(&co);
    cr_assert_eq(
        result,
        0,
        "Polling after stopping should not change coroutine state."
    );
}

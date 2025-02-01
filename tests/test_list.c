#include "unity.h"
#include "list.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}


struct node {
    struct list_head list;
    int a;
};

void test_list_add_del(void) {
    LIST_HEAD(head);

    struct node n1 = { .a = 1, .list = LIST_HEAD_INIT(n1.list) };
    struct node n2 = { .a = 2, .list = LIST_HEAD_INIT(n2.list) };
    struct node n3 = { .a = 3, .list = LIST_HEAD_INIT(n3.list) };

    list_add(&n1.list, &head);
    list_add(&n2.list, &head);
    list_add(&n3.list, &head);

    struct node *iter;
    int i = 3;
    list_for_each_entry(iter, &head, list) {
        TEST_ASSERT_EQUAL_INT(i, iter->a);
        i -= 1;
    }
    list_del(&n1.list);
    list_del(&n2.list);
    list_del(&n3.list);

    TEST_ASSERT_TRUE(list_empty(&head));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_list_add_del);
    return UNITY_END();
}

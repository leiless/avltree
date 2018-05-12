#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "avltree.h"

static void *dummy = (void *) "0cea5b81-ec30-4efb-b505-c08c0aac7564";

void test1(void)
{
    fprintf(stderr, "%s\n", __func__);

    avltree_t *t = avltree_alloc();
    if (t == NULL) {
        perror("avltree_alloc fail");
        return;
    }

    avltree_show(t);
    assert(avltree_getsize(t) == 0);

    avltree_clear(t);
    assert(avltree_getsize(t) == 0);

    assert(avltree_find(t, dummy, 0) == 0);

    assert(avltree_delete(t, dummy, 0) == ENOENT);

    avltree_free(&t);

#if 0
    avltree_show(t);    /* Use after free  should fail anyway */
#endif
}

void loselose_srand(void)
{
    struct timeval tv;
    unsigned long junk = 0xdeadbeef;
    uintptr_t p = (uintptr_t) &junk;
    gettimeofday(&tv, NULL);
    srand((getpid() << 16) ^ tv.tv_sec ^ tv.tv_usec ^ (p & 0xffff00));
}

void test2(void)
{
    fprintf(stderr, "%s\n", __func__);

    avltree_t *t = avltree_alloc();
    if (t == NULL) {
        perror("avltree_alloc fail");
        return;
    }

    loselose_srand();

    size_t i;
    int val;

    size_t ok = 0;
    size_t exist = 0;
    size_t nomem = 0;

#define TEST_SIZE 50000
    for (i = 0; i < TEST_SIZE; i++) {
#define RAND_RANGE 200000
        val = rand() % RAND_RANGE;
        switch (avltree_insert(t, (unsigned char *) &val, sizeof(val))) {
        case 0:
            ok++;
            break;
        case EEXIST:
            exist++;
            break;
        case ENOMEM:
            nomem++;
            break;
        default:
            assert(((void) "Unswitched return value", 0));
            break;
        }
    }

    fprintf(stderr, "OK: %zu EXIST: %zu NOMEM: %zu\n", ok, exist, nomem);

    size_t found = 0;
    size_t not_found = 0;
    for (i = 0; i < TEST_SIZE; i++) {
        val = rand() % RAND_RANGE;
        avltree_find(t, (unsigned char *) &val, sizeof(val))
            ? found++ : not_found++;
    }

    fprintf(stderr, "FOUND: %zu NOT: %zu\n", found, not_found);

    size_t del = 0;
    for (i = 0; i < TEST_SIZE * 2; i++) {
        val = rand() % RAND_RANGE;
        del += !avltree_delete(t, (unsigned char *) &val, sizeof(val));
    }

    fprintf(stderr, "DELETED: %zu\n", del);
    fprintf(stderr, "Current tree size: %u\n", avltree_getsize(t));

    assert(avltree_getsize(t) + del == ok);

    avltree_clear(t);
    assert(avltree_getsize(t) == 0);
    avltree_show(t);

    avltree_free(&t);
    assert(t == NULL);
}

void test3(void)
{
    fprintf(stderr, "%s\n", __func__);

    /*
     * TODO: using char * as tree value
     */
}

int main(void)
{
    test1();
    test2();
    test3();
    puts("Tests passed");
    return 0;
}


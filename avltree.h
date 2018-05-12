/*
 * Created 18A11
 *
 * An ANSI bottom-up AVL tree implementation
 */

#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define __malloc(sz)    malloc(sz)
#define __free(ptr)     free(ptr)
#define __assert__      assert

struct avltree_node {
    struct avltree_node *lchild;
    struct avltree_node *rchild;
    uint32_t height;
    uint32_t size;
    unsigned char data[];         /* zero length array */
};

typedef struct avltree_node avltree_node_t;

struct avltree {
    avltree_node_t *root;
    uint32_t size;
};

typedef struct avltree avltree_t;

avltree_t *avltree_alloc(void);
uint32_t avltree_free(avltree_t **);
uint32_t avltree_clear(avltree_t *);
uint32_t avltree_getsize(const avltree_t *);
int avltree_find(const avltree_t *, const unsigned char *, uint32_t);
int avltree_insert(avltree_t *, const unsigned char *, uint32_t);
int avltree_delete(avltree_t *, const unsigned char *, uint32_t);
void avltree_show(const avltree_t *);

#endif


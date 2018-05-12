/*
 * Created 18A11
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "avltree.h"

static uint32_t umax(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

static avltree_node_t *avltree_node_alloc(const unsigned char *v, uint32_t sz)
{
    avltree_node_t *n;

    n = __malloc(sizeof(*n) + sz);
    if (n == NULL)
        goto out_exit;

    n->lchild = NULL;
    n->rchild = NULL;
    n->height = 1;      /* leaf node heighted 1 */
    n->size = sz;
    memcpy(n->data, v, sz);

out_exit:
    return n;
}

/**
 * Allocate an AVL tree
 * @return          a pointer to new allocated tree if success  NULL o.w.
 */
avltree_t *avltree_alloc(void)
{
    avltree_t *t = __malloc(sizeof(*t));
    if (t == NULL)
        goto out_exit;

    t->root = NULL;
    t->size = 0;

out_exit:
    return t;
}

static uint32_t node_height(const avltree_node_t *);

static uint32_t node_balance_check(const avltree_node_t *n)
{
    if (n == NULL) return 0;

    __assert__(n->height ==
        1 + umax(node_height(n->lchild), node_height(n->rchild)));

    if (n->lchild && n->rchild) {
        return 1 +
            node_balance_check(n->lchild) + node_balance_check(n->rchild);
    } else if (n->lchild || n->rchild) {
        return 1 +
            node_balance_check(n->lchild ? n->lchild : n->rchild);
    }

    return 1;
}

/*
 * Check AVL tree sanity(mainly for debugging)
 */
static void avltree_balance_check(const avltree_t *t)
{
    __assert__(t != NULL);
    __assert__(node_balance_check(t->root) == t->size);
}

/*
 * In-order tail recursion optimized
 */
static void avltree_node_free_recur(avltree_t *t, avltree_node_t *n)
{
out_refine:
    if (n == NULL) return;

    if (n->lchild)
        avltree_node_free_recur(t, n->lchild);

    if (n->rchild) {
        avltree_node_t *tmp = n->rchild;
        __free(n);
        t->size--;
        n = tmp;
        goto out_refine;
    }

    __free(n);
    t->size--;
}

/*
 * Deallocate the AVL tree
 * @param t     the tree
 * @param full  if free the tree entirely
 * @return      # of elements before deallocate
 */
static uint32_t avltree_dealloc(avltree_t *t, int full)
{
    uint32_t size;
    __assert__(t != NULL);
    size = t->size;

    avltree_balance_check(t);

    avltree_node_free_recur(t, t->root);
    __assert__(t->size == 0);
    t->root = NULL;
    if (full) __free(t);

    return size;
}

uint32_t avltree_free(avltree_t **t)
{
    __assert__(t != NULL);
    uint32_t sz = avltree_dealloc(*t, 1);
    *t = NULL;
    return sz;
}

/*
 * Delete all items inside the tree  yet preserve an empty tree
 */
uint32_t avltree_clear(avltree_t *t)
{
    return avltree_dealloc(t, 0);
}

uint32_t avltree_getsize(const avltree_t *t)
{
    __assert__(t != NULL);
    return t->size;
}

static int node_bcmp(const avltree_node_t *n, const unsigned char *v, uint32_t sz)
{
    __assert__(n != NULL && v != NULL);
    if (sz != n->size)
        return sz < n->size ? -1 : 1;
    return memcmp(v, n->data, sz);
}

/*
 * Find a value in the AVL tree
 *
 * @param t     the tree
 * @param v     the value to be find
 * @param sz    size of the value(in bytes)
 * @return      1 if the value found in the tree  0 o.w.
 */
int avltree_find(const avltree_t *t, const unsigned char *v, uint32_t sz)
{
    long i;
    avltree_node_t *n;

    __assert__(t != NULL && v != NULL);
    n = t->root;

    while (n != NULL) {
        i = node_bcmp(n, v, sz);
        if (i == 0)
            return 1;
        n = i < 0 ? n->lchild : n->rchild;
    }

    return 0;
}

static uint32_t node_height(const avltree_node_t *n)
{
    return n ? n->height : 0;
}

static int32_t node_getbalance(const avltree_node_t *n)
{
    return n ? node_height(n->lchild) - node_height(n->rchild) : 0;
}

/*
 *          root
 *      lchild
 */
static avltree_node_t *rot_ll(avltree_node_t *r)
{
    avltree_node_t *lc = r->lchild;
    r->lchild = lc->rchild;
    lc->rchild = r;

    r->height = 1 + umax(node_height(r->lchild), node_height(r->rchild));
    lc->height = 1 + umax(node_height(lc->lchild), r->height);

    return lc;
}

/*
 *          root
 *              rchild
 */
static avltree_node_t *rot_rr(avltree_node_t *r)
{
    avltree_node_t *rc = r->rchild;
    r->rchild = rc->lchild;
    rc->lchild = r;

    r->height = 1 + umax(node_height(r->lchild), node_height(r->rchild));
    rc->height = 1 + umax(r->height, node_height(rc->rchild));

    return rc;
}

/*
 *              root
 *      lchild
 *          rchild
 */
static avltree_node_t *rot_lr(avltree_node_t *root)
{
    root->lchild = rot_rr(root->lchild);
    return rot_ll(root);
}

/*
 *              root
 *                      rchild
 *                  lchild
 */
static avltree_node_t *rot_rl(avltree_node_t *root)
{
    root->rchild = rot_ll(root->rchild);
    return rot_rr(root);
}

static avltree_node_t *avltree_insert_recur(avltree_node_t *n,
                                            const unsigned char *v,
                                            uint32_t sz)
{
    int i;

    if (n == NULL) {
        n = avltree_node_alloc(v, sz);
        if (n == NULL)
            errno = ENOMEM;
        goto out_exit;
    }

    i = node_bcmp(n, v, sz);
    if (i == 0) {
        errno = EEXIST;
        goto out_exit;
    }

    if (i < 0) {
        n->lchild = avltree_insert_recur(n->lchild, v, sz);
        if (node_getbalance(n) > 1) {
            if (node_bcmp(n->lchild, v, sz) < 0)
                return rot_ll(n);
            return rot_lr(n);
        }
    } else {
        n->rchild = avltree_insert_recur(n->rchild, v, sz);
        if (node_getbalance(n) < -1) {
            if (node_bcmp(n->rchild, v, sz) > 0)
                return rot_rr(n);
            return rot_rl(n);
        }
    }

    n->height = 1 + umax(node_height(n->lchild), node_height(n->rchild));

out_exit:
    return n;
}

/*
 * Insert a item into AVL tree
 * @return      0 if inserted  errno o.w.
 *              ENOMEM if of out memory
 *              EEXIST if the value already exists
 */
int avltree_insert(avltree_t *t, const unsigned char *v, uint32_t sz)
{
    __assert__(t != NULL && v != NULL);
    errno = 0;
    t->root = avltree_insert_recur(t->root, v, sz);
    if (errno == 0) {
        __assert__(t->root != NULL);
        t->size++;
        avltree_balance_check(t);
    }
    return errno;
}

static avltree_node_t *node_min(avltree_node_t *n)
{
    avltree_node_t *cursor = n;
    __assert__(n != NULL);
    while (cursor->lchild != NULL)
        cursor = cursor->lchild;
    return cursor;
}

static avltree_node_t *avltree_delete_recur(avltree_node_t *n,
                                            const unsigned char *v, uint32_t sz)
{
    long i;
    int32_t diff;

    if (n == NULL) {
        errno = ENOENT;
        goto out_exit;
    }

    i = node_bcmp(n, v, sz);
    if (i == 0) {
        if (n->lchild == NULL || n->rchild == NULL) {
            avltree_node_t *tmp = n->lchild ? n->lchild : n->rchild;
            __free(n);
            return tmp;
        } else {
            avltree_node_t *nc = n->rchild;
            avltree_node_t *min = node_min(nc);
            /*
             * realloc isn't versatile as malloc  thus we pursue malloc
             */
            avltree_node_t *tmp = __malloc(sizeof(*min) + min->size);
            if (tmp == NULL) {
                errno = ENOMEM;
                goto out_exit;
            }
            tmp->height = n->height;
            tmp->size = min->size;
            memcpy(tmp->data, min->data, min->size);
            tmp->lchild = n->lchild;
            __free(n);
            tmp->rchild = avltree_delete_recur(nc, min->data, min->size);
            n = tmp;
        }
    } else if (i < 0) {
        n->lchild = avltree_delete_recur(n->lchild, v, sz);
    } else {
        n->rchild = avltree_delete_recur(n->rchild, v, sz);
    }

    diff = node_getbalance(n);
    if (diff > 1) {
        if (node_getbalance(n->lchild) >= 0)
            return rot_ll(n);
        return rot_lr(n);
    } else if (diff < -1) {
        if (node_getbalance(n->rchild) <= 0)
            return rot_rr(n);
        return rot_rl(n);
    }

    n->height = 1 + umax(node_height(n->lchild), node_height(n->rchild));
out_exit:
    return n;
}

/*
 * Delete a item from AVL tree
 * @return      0 if deleted  errno o.w.
 *              ENOENT if no such value in the tree
 *              ENOMEM if out of memory(tree untouched)
 */
int avltree_delete(avltree_t *t, const unsigned char *v, uint32_t sz)
{
    __assert__(t != NULL && v != NULL);
    errno = 0;
    t->root = avltree_delete_recur(t->root, v, sz);
    if (errno == 0) {
        if (--t->size == 0)
            __assert__(t->root == NULL);
        avltree_balance_check(t);
    }
    return errno;
}

/*
 * You definitely should implement this on your own
 */
static void avltree_show_recur(const avltree_node_t *n)
{
out_reshow:
    if (n == NULL) return;

    if (n->lchild)
        avltree_show_recur(n->lchild);

    printf("%.*s ", (int) n->size, n->data);

    n = n->rchild;
    goto out_reshow;
}

/*
 * Display the AVL tree(for debugging)
 */
void avltree_show(const avltree_t *t)
{
    __assert__(t != NULL);
    if (t->root == NULL) {
        printf("empty AVL tree %p\n", t);
    } else {
        avltree_show_recur(t->root);
    }
}


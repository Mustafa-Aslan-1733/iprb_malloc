#include <stdlib.h>
#include <stdio.h>
#include "rb_malloc_tree.h"
#include "rb_malloc_tree_node.h"
#include "rb_malloc_tree_builder.h"

#define PAGE_SIZE 4096
#define WORD 8
#define DWORD 16

static void *sp, *bp, *ep, *max;
static long *tree;
static void *lb, *bb;

void init_mem(){
    sp = malloc(1024*1024);
    bp = sp;
    max = sp + 1024*1024;
    ep = sp + PAGE_SIZE;
    tree = 0;
    bp += WORD;
    lb = bp;
    bb = lb;
    bp += 4*WORD;
    set_size(lb, 4*WORD);
    init_block(lb);
    set_alloc(lb, 1);
    set_prev_alloc(lb, 1);
}

void extend(size_t size){
    size_t sz = (size+PAGE_SIZE-1)/PAGE_SIZE;
    if (ep + sz > ep){
        fprintf(stderr, "not enough memory!");
        return;
    }
    ep += sz;
}


void *rb_malloc(int sz){
    sz += 8;
    sz = (sz + 15)/DWORD*DWORD;
    if (sz == DWORD) sz = 2*DWORD;
    void *ptr = get_from_tree(&tree, sz);
    if (ptr){
        delete_node_by_pointer(&tree, ptr);
        int bsize = get_size(ptr);
        int dif = bsize - sz;
        if (dif >= 4*WORD){
            set_size(ptr, sz);
            void *nxt = get_next_block(ptr);
            set_size(nxt, dif);
            init_block(nxt);
            rb_tree_insert(&tree, nxt);
        }
        set_alloc(ptr, 1);
        void *nxt = get_next_block(ptr);
        set_prev_alloc(nxt, 1);
        return ptr+WORD;
    }

    //create new block
    ptr = bp;
    bp += sz;
    set_size(ptr, sz);
    init_block(ptr);
    set_alloc(ptr, 1);
    if (is_allocated(lb)){
        set_prev_alloc(ptr, 1);
    }    
    lb = ptr;
    return ptr+WORD;

}

void iterate(){
    void *cur = bb;
    if (cur == lb) return;
    do{
        fprintf(stderr, "%ld        %s %ld\n", get_size(cur)
                ,is_allocated(cur) ? "x":" ", cur);
        cur = get_next_block(cur);
    }while (cur != lb);
    fprintf(stderr, "%ld        %s %ld\n", get_size(cur)
            ,is_allocated(cur) ? "x":" ", cur);
}

void rb_free(void *ptr){
    ptr -= 8;
    void *nxt = get_next_block(ptr);
    if (nxt < bp){
        set_prev_alloc(nxt, 0);
    }

    int eb = (nxt >= bp);
    if (!prev_alloc(ptr) && (!eb && !is_allocated(nxt))){
        int size = get_size(nxt) + get_size(ptr) + get_size(ptr - WORD);
        void *h = get_header(ptr - WORD);
        delete_node_by_pointer(&tree, h);
        delete_node_by_pointer(&tree, nxt);
        set_size(h, size);
        if (lb == nxt || lb == ptr){
            lb = h;
        }
        ptr = h;
    }else if (!prev_alloc(ptr) && (eb || is_allocated(nxt))){
        int size = get_size(ptr) + get_size(ptr - WORD);
        void *h = get_header(ptr - WORD);
        delete_node_by_pointer(&tree, h);
        set_size(h, size);
        if (lb == ptr){
            lb = h;
        }
        ptr = h;
    }else if (prev_alloc(ptr) && (!eb && !is_allocated(nxt))){
        int size = get_size(ptr) + get_size(nxt);
        delete_node_by_pointer(&tree, nxt);
        set_size(ptr, size);
        if (lb == nxt) lb = ptr;
    }else
    init_block(ptr);
    set_alloc(ptr, 0);
    rb_tree_insert(&tree, ptr);
}


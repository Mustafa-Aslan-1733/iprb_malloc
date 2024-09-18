#include "rb_malloc_tree_node.h"
#include <stdio.h>

int prev_alloc(long *header){
    return ((*header)&2) ? 1:0;
}

int get_color(long *header){
    return ((*header)&8) ? 1:0;
}

int is_allocated(long *header){
    return (*header)&1;
}

unsigned get_size(long *both){
    if ((*both)&4){
        return 32;
    }else{
        return (*both) & ~0xf;
    }
}

int is_rb(long *footer){
    return ((*footer)&2) ? 1:0;
}

void set_prev_alloc(long *header, int y){
    if ((*header & 0x2) != y<<1) *header ^= 0x2;
}

void set_color(long *header, int color){
    if ((*header & 0x8) != color<<3) (*header)^=0x8;
}

void set_alloc(long *header, int y){
    if ((*header & 0x1) != y) (*header)^=0x1;
}

//always call this function first before the other ones, this sets the 
//boundaries (footer) of the block
void set_size(long *header, unsigned size){
    if (size == 4 * WORD){
        (*header)|=4; 
        header += 3;
        (*header)|=4; 
    }else{
        long hf = (*header)&0x7;
        (* header) = size;
        (*header)|=hf;
        (*header)&=~0x4;
        header += size/8 - 1;
        (* header) = size;
    }
}

void set_is_rb(long *footer, int y){
    if ((*footer & 0x2) != y<<1) (*footer)^=0x2;
}

void *get_parent(long *header){
    if (get_size(header) != 4 * WORD){
        header ++;
    }
    if (!((*header) & ~0xf)) return 0;
    return (void*)((*header) & ~0xf | 0x8);
}


void set_parent(long *header, long *parent){
    parent = (long*)((long)parent & ~0x8);
    if (get_size(header) != 4 * WORD){
        header ++;
    }
    long flags = (*header) & 0xf;
    *header = (long)parent | flags;
}

void set_child(long *header, long *child, int dir){
    child = (long*)((long)child & ~0x8);
    unsigned sz = get_size(header);
    if (sz != 4 * WORD){
        header += 3+dir;
    }else{
        header += 2+dir;
    }
    long flags = (*header) & 0xf;
    *header = (long)child;
    (*header) |= flags;
}

void *get_child(long *header, int dir){
    if (get_size(header) != 4 * WORD)
        header += 3+dir;
    else
        header += 2+dir;
    //fix it in the future
    if (!((*header) & ~0xf)) return 0;
    return (void*)((*header) & ~0xf | 0x8);
}

void set_child_is_leaf(long *header, int dir, int leaf){
    if (get_size(header) != 4 * WORD)
        header += 3+dir;
    else
        header += 2+dir;
    if (((*header) & 0x1) != leaf) (*header) ^= 0x1;
}

int child_is_leaf(long *header, int dir){
    if (get_size(header) != 4 * WORD)
        header += 3+dir;
    else
        header += 2+dir;
    return (*header) & 0x1;
}

//this assumes that no flags are stored in the lower order bytes of the key
//variable
long *get_list(long *header){
    if (get_size(header) != 4*WORD)
        header += 2;
    else
        header ++;
    return (long*)*header;
}

void set_list(long *header, long *dl){
    if (get_size(header) != 4*WORD)
        header += 2;
    else
        header ++;
    *header = (long)dl;
}

void *get_footer(long *header){
    return header + (get_size(header)/8 - 1);
}

void *get_header(long *footer){
    return footer - (get_size(footer)/8 - 1);
}

//call after setting the size with set_size, that function sets the boundaries
//of the blocks
void init_block(long *header){
    long hf = (*header)&0x7;
    long fflags = 0;
    if (get_size(header) == 4 * WORD){
        *header &= 0xf;
        fflags = *(header + 3) & 0x4;
        header++;
    }else{
        header ++;
        *header = 0;
        header ++;
    }
    *header = 0;
    header ++;
    *header = 0;
    header ++;
    *header = 0;
    if (fflags) *header |= fflags;
    (*header)|=hf;
}

// these functions are designed for linked list nodes
long *get_prev(long *header){
    if (get_size(header) != 4*WORD) header++;
    return (long*)(*(header+1));
}

long *get_next(long *header){
    if (get_size(header) != 4*WORD) header++;
    return (long*)(*(header+2));
}

void set_prev(long *header, long *val){
    if (get_size(header) != 4*WORD) header++;
    *(header+1) = (long)val;
}

void set_next(long *header, long *val){
    if (get_size(header) != 4*WORD) header++;
    *(header+2) = (long)val;
}

void set_rb_root(long *header, long *root){
    long *f = get_footer(header);
    long val = ((long)root) & ~0xf;
    *f = ((*f) & ~0xf | val);
}

long *get_rb_root(long *header){
    long *f = get_footer(header);
    long val = (*f) & ~0xf;
    return (long*)(val|0x8);
}
// </> these functions are designed for linked list nodes

void move_rb_node(long *src, long *dst){
    long hflags, *parent, key, *lchild, *rchild, fflags;
    hflags = (*src) & 0xb;
    parent = get_parent(src);
    lchild = get_child(src, 0);
    rchild = get_child(src, 1);
    fflags = (*(long*)get_footer(src)) & 0xb;

    set_list(dst, get_list(src));
    set_parent(dst, parent);
    set_is_rb(get_footer(dst), is_rb(get_footer(src)));
    set_color(dst, get_color(src));
    set_child(dst, lchild, 0);
    set_child(dst, rchild, 1);
    long *footer = get_footer(dst);
    (*footer) = (*footer) & ~0xb | fflags;
    (*dst) = (*dst) & ~0xb | hflags;
}

long *get_next_block(long *node){
    return node + get_size(node)/8;
}

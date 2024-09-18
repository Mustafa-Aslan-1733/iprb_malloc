#include <stdlib.h>
#include <stdio.h>
#include "rb_malloc_tree.h"
#include "rb_malloc_tree_node.h"
#include "rb_malloc_tree_builder.h"

static void delete_node(long **tree, int val, long **reb, long **rep);

void *get_from_tree(long **tree, int req){
    void *node = *tree;
    if (!*tree || !get_size(*tree)) return NULL;
    int found = 1;

    unsigned key = get_size(node);
    while(key != req){
        void *child = get_child(node, (key<req));
        if (!child){
            found = 0;
            break;
        }
        node = child;
        key = get_size(node);
    }

    if (!found && get_size(node) <= req) {
        void *p = get_parent(node);
        while(p){
            int dir = (get_child(p, 0) == node)? 0:1;
            node = p;
            if (dir == 0) break;
        }
        if (!p) return NULL;
    }
    req = get_size(node);
    void *dlh = get_list(node);
    if (!dlh){
        //change later
        rb_tree_delete(tree, req);
        return node;
    }
    void *nxt = get_next(dlh);
    if (nxt == dlh){
        set_list(node, NULL);
        return dlh;
    }
    void *nnxt = get_next(nxt);
    set_next(dlh, nnxt);
    set_prev(nnxt, dlh);
    //node + WORD
    return nxt;
}

static void insert_to_dl_list(long *cur, long *node){
    void *dlh = get_list(cur);
    set_is_rb(get_footer(node), 0);
    if (!dlh){
        set_list(cur, node);
        set_parent(node, cur);
        set_next(node, node);
        set_prev(node, node);

        return;
    }
    
    void *nxt = get_next(dlh);
    set_next(dlh, node);
    set_prev(node, dlh);
    set_prev(nxt, node);
    set_next(node, nxt);
}


static int insert(long **root, long *node){
    long *cur = *root;
    if (!cur){
        *root = node;
        set_is_rb(get_footer(node), 1);
        return 0;
    }

    long cur_key = get_size(cur), node_key = get_size(node);
    int c_dir;
    while (cur_key != node_key){
        c_dir = (cur_key < node_key);
        long *child = get_child(cur, c_dir);
        if (child){
            cur = child;
            cur_key = get_size(cur);
            continue;
        }
        //not found

        set_child(cur, node, c_dir);
        set_parent(node, cur);

        set_is_rb(get_footer(node), 1);
        set_color(node, RED);
        return 1;
    }
    //already exists
    
    insert_to_dl_list(cur, node);
    return 0;
}

//this needs to be static
void getpgu(long *node, long **p, long **g, long **u, int *dir){
    *p = get_parent(node);
    if (!*p || !get_parent(*p)) {
        *g = *u = NULL;
        return;
    }
    *g = get_parent(*p);
    //leaf child and NULL node think about it
    if (get_child(*g, 1) == (*p)){
        *u = get_child(*g, 0);
        *dir = 1;
    }else if (get_child(*g, 0) == (*p)){
        *u = get_child(*g, 1);
        *dir = 0;
    }
}

static void rotate(long *c, long *p, int dir){
    long *g = get_parent(p);
    if (g){
        int gd = (get_child(g, 0) == p) ? 0:1;
        set_child(g, c, gd);
    }
    set_parent(c, g);
    long *sc = get_child(c, 1-dir);
    set_child(p, sc, dir);
    if (sc){
        set_parent(sc, p);
    }

    set_child(c, p, 1-dir);
    set_parent(p, c);
}

static int is_black(long *node){
    return !node || get_color(node) == BLACK;
}

static int is_red(long *node){
    return node && get_color(node) == RED;
}

static void i_rebalance(long **tree, long *node){
    if (!get_parent(node)) return;
    int dir;
    long *p,*g,*u;
    getpgu(node, &p, &g, &u, &dir);
    while(1){
        //I3;
        if (!p) return;
        //I1
        if (is_black(p)) return;
        //I4
        if (is_red(p) && !g){
            set_color(p, BLACK);
            return;
        }
        //I2
        if (is_red(p) && is_red(u)){
            set_color(p, BLACK);
            set_color(u, BLACK);
            set_color(g, RED);
            node = g;
            getpgu(node, &p, &g, &u, &dir);
            continue;
        }
        //I5
        int dir = (get_child(p, 0) == node) ? 0:1;
        if (get_child(g, 1-dir) == p){
            rotate(node, p, dir);
            node = p;
            getpgu(node, &p, &g, &u, &dir);
        }
        //I6
        dir = (get_child(g, 0) == p) ? 0:1;
        rotate(p, g, dir);
        set_color(p, BLACK);
        set_color(g, RED);
        if (!get_parent(p)) {
            *tree = p;
        }
        return;
    }
}

void rb_tree_insert(long **root, long *node){
    if (insert(root, node))
        i_rebalance(root, node);
}

static void move_node(long *node, long *rep, long **tree){
    if (node == rep){
        return;
    }

    move_rb_node(rep, node);
    void *p = get_parent(node);

    if (!p){
        *tree = node;
    }else{
        int dir = (get_child(p, 0) == rep)? 0:1;
        set_child(p, node, dir);
    }
    void *c1, *c2;
    c1 = get_child(node, 0), c2 = get_child(node, 1);

    if (c1){
        set_parent(c1, node);
    }
    if (c2){
        set_parent(c2, node);
    }
    void *dlh = get_list(node);
    if (dlh){
        set_parent(dlh, node);
    }
    //free(node);
}

static void delete_node(long **tree, int val, long **reb, long **rep){
    long *cur = *tree;
    int key = get_size(cur);
    while (key != val){
        long *child = get_child(cur, (key < val));
        if (!child){
            *reb = NULL;
            return;
        }
        cur = child;
        key = get_size(cur);
    }

    *rep = cur;
    //simple cases
    void *c1 = get_child(cur, 0), *c2 = get_child(cur, 1); 
    if (c1 && c2){
        long *swap = get_child(cur, 1);
        void *lchild = get_child(swap, 0);
        while(lchild){
            swap = lchild;
            lchild = get_child(swap, 0);
        }
        cur = swap;
    }

    c1 = get_child(cur, 0), c2 = get_child(cur, 1); 
    if (c1){
        //cur == rep
        void *oc1 = get_child(c1, 0);
        void *oc2 = get_child(c1, 1);

        set_child(cur, oc1, 0);
        set_child(cur, oc2, 1);
        set_color(cur, BLACK);
        *reb = NULL;
        move_node(c1, *rep, tree);
        return;
    }

    if (c2){
        if (*rep == cur){
            void *p = get_parent(cur);
            if (p){
                int dir = (get_child(get_parent(cur), 0) == cur)?0:1;
                set_child(p, c2, dir);
                set_parent(c2, p);
            }else{
                *tree = c2;
                set_child(c2, 0, 0);
                set_parent(c2, 0);
            }
        }else if (get_parent(cur) == *rep){
            set_child(*rep, c2, 1);
        }else{
            void *p = get_parent(cur);
            set_child(p, c2, 0);
            set_parent(c2, p);
        }
        set_color(c2, BLACK);
        *reb = NULL;
        move_node(cur, *rep, tree);
        return;
    }

    if (!get_parent(cur)){
        *tree = NULL;
        *reb = NULL;
        return;
    }

    if (is_red(cur)){
        void *oc = get_child(cur, (cur != *rep));
        void *p = get_parent(cur);
        int dir = (get_child(p, 0) == cur) ? 0 : 1;
        set_child(p, oc, dir);
        if (oc){
            set_parent(oc, p);
        }
        move_node(cur, *rep, tree);
        *reb = NULL;
        return;
    }
    void *p = get_parent(cur);
    int dir = (get_child(get_parent(cur), 0) == cur)? 0:1;
    c2 = dir?c1:c2;
    set_child(p, c2, dir);
    if (c2){
        set_parent(c2, p);
    }
    *reb = cur;
}

int tree_has_key(long *tree, int tar){
    if (!tree) return 0;
    long *node = tree;
    int key = get_size(node);
    while(tar != key){
        void *child = get_child(node, key<tar);
        if (!child) return 0;
        node = child;
        key = get_size(node);
    }
    return 1;
}

void delete_node_by_pointer(long **tree, long *node){
    void *par;
    if (is_rb(get_footer(node))){
        void *dst = get_from_tree(tree, get_size(node));
        if (!tree_has_key(*tree, get_size(node)))
            return;
        else{
            move_node(dst, node, tree);
       }
    }else if ((par = get_parent(node))){
        //this is a linked list header
        long *nxt = get_next(node);

        if (nxt == node){
            set_list(par, NULL);
        }else{
            long *pre = get_prev(node);
            set_next(pre, nxt);
            set_prev(nxt, pre);
            set_parent(nxt, get_parent(node));
            set_list(par, nxt);
        }
    }else{
        long *nxt = get_next(node);
        long *pre = get_prev(node);
        set_prev(nxt, pre);
        set_next(pre, nxt);
    }
}

static void getpscd(long *node, long **p, long **s,
        long **c, long **d, int *dr){
    if (!get_parent(node)){
        *p = NULL;
        *c = NULL;
        *d = NULL;
        return;
    }

    *p = get_parent(node);
    int dir = (get_child(*p, 0) == node) ? 0:1;
    *s = get_child(*p, 1-dir);
    *c = get_child(*s, dir);
    *d = get_child(*s, 1-dir);
    *dr = dir;
}

static void d_rebalance(long **tree, long *node, long *rep){
    long *p, *s, *c, *d;
    int dir;
    getpscd(node, &p, &s, &c, &d, &dir);
    move_node(node, rep, tree);
    if (p == rep) p = node;

    while(1){
        //D1
        if (!p) {
            return;
        }
        //D2
        if (is_black(p) && is_black(c) && 
                is_black(s) && is_black(d)){
            set_color(s, RED);
            node = p;
            getpscd(node, &p, &s, &c, &d, &dir);
            continue;
        }
        //D3
        if (is_black(p) && is_red(s)){
            int dir = (get_child(p, 0) == s) ? 0 : 1;
            rotate(s, p, dir);
            set_color(p, RED);
            set_color(s, BLACK);
            if (*tree == p) *tree = s;
            getpscd(node, &p, &s, &c, &d, &dir);
        }
        //D4
        if (is_red(p) && is_black(s) && is_black(c) && is_black(d)){
            set_color(p, BLACK);
            set_color(s, RED);
            return;
        }
        //D5
        if (is_red(c)){
            int dir = (get_child(s, 0) == c) ? 0:1;
            rotate(c, s, dir);
            set_color(c, BLACK);
            set_color(s, RED);
            getpscd(node, &p, &s, &c, &d, &dir);
        }

        //D6
        int dir = (get_child(p, 0) == s) ? 0:1;
        rotate(s, p, dir);
        if (!get_parent(s)) *tree = s;
        set_color(s, get_color(p));
        set_color(p, BLACK);
        set_color(d, BLACK);
        return;
    }
}

void rb_tree_delete(long **tree, int val){
    long *reb, *rep;
    delete_node(tree, val, &reb, &rep);
    if (reb) d_rebalance(tree, reb, rep);
}

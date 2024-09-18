#include <stdio.h>
#include "rb_malloc_tree.h"
#include "rb_malloc_tree_builder.h"
#include "rb_malloc_tree_node.h"
#include "rb_malloc.h"

void getpgu(long *node, long **p, long **g, long **u);
void rotate(long *c, long *p, int dir);
void gprv(void *ptr);
void ptree();
void iterate();

int main(){
    init_mem();
    long *p = 0;
    //long *p = get_rb_malloc_tree(buf);
    //fprintf(stderr, "out\n");
    //print_levels(p);
    while (1){
        char op;
        scanf("%c", &op);
        if (op == '\n') continue;
        if (op == 'i'){
            int key;
            scanf("%d", &key);
            key = key >= 3 ? key:3;
            void *n = rb_malloc_ntb(key*16-8);
            n-=8;
            fprintf(stderr, "node to be inserted: %ld\n", n);
            rb_tree_insert(&p, n);
            print_levels(p);
        }else if (op == 'g'){
            long sz;
            scanf("%ld", &sz);
            void *res = get_from_tree(&p, sz*16);
            if (!res) fprintf(stderr, "no matching block\n");
            else fprintf(stderr, "size: %ld\n", get_size(res));
        }if (op == 'd'){
            int val;
            scanf("%d", &val);
            rb_tree_delete(&p, val*16);
            print_levels(p);
        }else if (op == 'p'){
            ptree();
        }else if (op == 's'){
            long p;
            char c;
            scanf("%ld %c", &p, &c);
            if (c == 'r')
                set_color((long*)p, RED);
            else
                set_color((long*)p, BLACK);
        }else if (op == 'a'){
            long ptr;
            scanf("%ld", &ptr);
            delete_node_by_pointer(&p, (long*)ptr);
        }else if (op == 'f'){
            long ptr;
            scanf("%ld", &ptr);
            rb_free((long*)ptr);
        }else if (op == 'm'){
            int key;
            scanf("%d", &key);
            void *n = rb_malloc(key*16);
            n-=8;
            fprintf(stderr, "node allocated: %ld\n", n);
        }else if (op=='c'){
            long ptr;
            scanf("%ld", &ptr);
            fprintf(stderr, "prev alloc:%d\n", prev_alloc((long*)ptr));
        }else{
            puts("not valid.");
        }
        iterate();
    }

}

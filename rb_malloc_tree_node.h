#define WORD 8

#define RED 0
#define BLACK 1

//block size and flag setters and getters
int prev_alloc(long *header);
int get_color(long *header);
int is_allocated(long *header);
unsigned get_size(long *both);
int is_rb(long *footer);
//int child_dir(long *footer);
void set_prev_alloc(long *header, int y);
void set_color(long *header, int color);
void set_alloc(long *header, int y);
void set_size(long *both, unsigned size);
void set_is_rb(long *footer, int y);
//void set_child_dir(long *footer, int dir);

//rb tree parameter setters and getters
void *get_parent(long *header);
void *get_child(long *header, int dir);
void set_parent(long *header, long *parent);
void set_child(long *header, long *child, int dir);
long *get_list(long *header);
void set_list(long *header, long *dl);

void *get_footer(long *header);
void *get_header(long *footer);

void init_block(long *header);

void set_prev(long *header, long *val);
void set_next(long *header, long *val);

long *get_prev(long *header);
long *get_next(long *header);

void set_rb_root(long *header, long *root);
long *get_rb_root(long *header);

void move_rb_node(long *src, long *dst);

long *get_next_block(long *node);

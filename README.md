# iprb_malloc
an in place implementation of malloc with red black tree

this may have bewn done before, but I put the nodes of the red black tree inside the free blocks, so that the algorithm doesn't take up any additional memory for storing the nodes (except a few variables, but probably some of those can be omitted too.

This is not meant to be a perfect implementation of this algorithm, this is my solution to the homework problem in the CSAPP Chapter Virtual memory last assignment of writing your own free and malloc functions.

There probably are bugs in the code but it is supposed to work fine with O(logN) malloc and free. 

The algorithm manages to fit the nodes inside the blocks by simply writing the node to the words 2-5 of the block, or by writing to the words 1-4 and setting some special flags in the 3rd bit of the header and footer word, indicating that this block has the minimal size of 32 bytes (4 words).

The size classes are stored in red black nodes and blocks of same size are stored in the doubly linked list pointed by the the node of the size class (except the block that is holding that node) as usually.

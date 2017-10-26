#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
//user included header
#include "structure.h"
#include "page.h"

#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
extern int order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
extern node * queue;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
extern bool verbose_output;


// FUNCTION PROTOTYPES
node* find_leaf(node *root, off_t *page_loc, int64_t key, bool verbose);
record* find(node *root, int64_t key, bool verbose);
int cut(int length);

// record* make_record(int64_t key, char* value, off_t writeat);
node* make_node(off_t *writeat);
node* make_leaf(off_t *leaf_loc);

node* insert_into_leaf(node *leaf, off_t writeat, int64_t key, char *value);
node* insert_into_leaf_after_splitting(node *root, node *leaf, off_t writeat, int64_t key, record *pointer);
node* start_new_tree(int64_t key, char *value);
node* insert(node *root, int64_t key, char *value);

#endif /* __BPT_H__*/
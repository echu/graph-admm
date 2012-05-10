#ifndef GADMM_H
#define GADMM_H

#define DATA_TYPE float

struct gadmm_factor;
struct gadmm_node;
struct gadmm_edge;

// // the user must define the following functions
// // compile-time error otherwise
// extern DATA_TYPE objective(struct gadmm_device *d);
// extern void solve(struct gadmm_device *d, const DATA_TYPE rho);

// creates a new node
struct gadmm_node *create_node(int T, int num_terms,
  DATA_TYPE (*objective_func)(struct gadmm_node*),
  void (*solver_func)(struct gadmm_node*, const DATA_TYPE));
  
// creates a new edge (everything initialized to 0)
struct gadmm_edge *create_edge(int len);

// creates a new factor (connections initialized to NULL)
struct gadmm_factor *create_factor(int len);

// frees the memory
void free_node(struct gadmm_node *d);
void free_edge(struct gadmm_edge *e);
void free_factor(struct gadmm_factor *f);

// add neighbors to factor 
// XXX: this may be implementation specific...
void add_neighbor(struct gadmm_factor *f, struct gadmm_edge *e);

// update the node
void update_node(struct gadmm_node *d);

// solve
// XXX: maybe push rho on to terminal level
void solve_node(struct gadmm_node *d, const DATA_TYPE rho);

// evaluate the objective on the node
DATA_TYPE evaluate_node(struct gadmm_node *d);

// update the terminal (fixed rho in this version)
//void update_edge(struct gadmm_edge *e, const DATA_TYPE *pbar);

// these are mutable
struct gadmm_edge *get_edge(struct gadmm_node *d, const int edge);
struct gadmm_factor *get_factor(struct gadmm_node *d, const int edge);

//DATA_TYPE get_p(const struct gadmm_edge* e, const int t);
//DATA_TYPE get_u(const struct gadmm_edge* e, const int t);
DATA_TYPE get_p(const struct gadmm_node *d, const int edge, const int t);
DATA_TYPE get_u(const struct gadmm_node *d, const int edge, const int t);

//void set_p(struct gadmm_edge* e, const int t, const DATA_TYPE p);
//void set_u(struct gadmm_edge* e, const int t, const DATA_TYPE u);
void set_p(struct gadmm_node *d, const int edge, const int t, const DATA_TYPE p);
void set_u(struct gadmm_node *d, const int edge, const int t, const DATA_TYPE u);

// get the number of edges out of node
int num_edges(const struct gadmm_node *d);
// get the length of the variable
int edge_length(const struct gadmm_node *d, const int edge);

#endif // GADMM_H
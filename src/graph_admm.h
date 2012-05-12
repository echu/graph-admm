#ifndef GADMM_H
#define GADMM_H

#define DATA_TYPE float

struct gadmm_vertex;
struct gadmm_edge;

typedef DATA_TYPE (*objective_func)(struct gadmm_vertex*, void*);
typedef void (*solver_func)(struct gadmm_vertex*, const DATA_TYPE, void*);

// creates a new vertex
// functions take void* to pass parameters
struct gadmm_vertex *create_vertex(int len, int num_terms,
  objective_func f, solver_func prox_f);
  
// creates a new edge (everything initialized to 0)
struct gadmm_edge *create_edge(int len);

// frees the memory
void free_vertex(struct gadmm_vertex *d);
void free_edge(struct gadmm_edge *e);

// solve
// XXX: maybe push rho on to terminal level
void solve_vertex(struct gadmm_vertex *d, const DATA_TYPE rho, void *params);

// evaluate the objective on the vertex
DATA_TYPE evaluate_vertex(struct gadmm_vertex *d, void *params);

// these are immmutable
const struct gadmm_edge *get_edge(struct gadmm_vertex *d, const int edge);
// these are mutable
struct gadmm_edge *get_mutable_edge(struct gadmm_vertex *d, const int edge);

DATA_TYPE get_p(const struct gadmm_edge* e, const int i);
DATA_TYPE get_u(const struct gadmm_edge* e, const int i);

void set_p(struct gadmm_edge* e, const int i, const DATA_TYPE p);
void set_u(struct gadmm_edge* e, const int i, const DATA_TYPE u);

// get the number of edges out of vertex
int num_edges(const struct gadmm_vertex *d);
// get the length of the edge
int edge_length(const struct gadmm_edge *e);

// connects two vertices through their given edge
// this function should be "override-able" or something
void connect(struct gadmm_vertex *d1, const int e1, struct gadmm_vertex *d2, const int e2);

#endif // GADMM_H
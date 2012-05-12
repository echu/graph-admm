#include "graph_admm.h"
#include <stdlib.h>

// an edge in the graph ADMM framework
struct gadmm_edge {
  int len;                // length of variable
  DATA_TYPE *p;           // variable
  DATA_TYPE *u;           // scaled dual variable
};

// a vertex in the graph ADMM framework
struct gadmm_vertex
{
  int num_edges;                		// number of edges
  struct gadmm_edge **edges;    		// my exposed edges
  
  // objective function pointer
  DATA_TYPE (*objective)(struct gadmm_vertex*);
  // solver function pointer
  void (*solve)(struct gadmm_vertex*, const DATA_TYPE);
};

// returns a device with the functions set
struct gadmm_vertex *create_vertex(int len, int num_edges,
  DATA_TYPE (*objective_func)(struct gadmm_vertex*),
  void (*solver_func)(struct gadmm_vertex*, const DATA_TYPE))
{
  struct gadmm_vertex *new_vertex = (struct gadmm_vertex *) malloc(sizeof(struct gadmm_vertex));
  new_vertex->num_edges = num_edges;

  new_vertex->edges = (struct gadmm_edge **) malloc(num_edges*sizeof(struct gadmm_edge *));
  
  for(int i = 0; i < num_edges; i++)
    new_vertex->edges[i] = create_edge(len);
  
  new_vertex->objective = objective_func;
  new_vertex->solve = solver_func;
  
  return new_vertex;
}

// returns a edge of length N with profiles and prices set to 0
struct gadmm_edge *create_edge(int len)
{
  struct gadmm_edge *new_edge = (struct gadmm_edge *) malloc(sizeof(struct gadmm_edge));
  new_edge->len = len;
  new_edge->p = (DATA_TYPE *) calloc(len, sizeof(DATA_TYPE));
  new_edge->u = (DATA_TYPE *) calloc(len, sizeof(DATA_TYPE));
  return new_edge;
}

// frees the memory
void free_vertex(struct gadmm_vertex *d)
{
  if(d) {
    if(d->edges) {
      for(int i = 0; i < d->num_edges; i++) 
        free_edge(d->edges[i]);
    }
    d->edges = NULL;
   
    free(d);
  }
  d = NULL;
}

// frees the memory
void free_edge(struct gadmm_edge *e)
{
  if(e) {
    if(e->p) free(e->p);
    e->p = NULL;
    if(e->u) free(e->u);
    e->u = NULL;
    free(e);
  }
  e = NULL;
}

// solve
void solve_vertex(struct gadmm_vertex *d, const DATA_TYPE rho)
{
  d->solve(d, rho);
}

// evaluate objective
inline DATA_TYPE evaluate_vertex(struct gadmm_vertex *d)
{
  return d->objective(d);
}

// accessors
inline const struct gadmm_edge* get_edge(struct gadmm_vertex *d, const int edge)
{
  // TODO: error checking
  return d->edges[edge];
}

inline struct gadmm_edge* get_mutable_edge(struct gadmm_vertex *d, const int edge)
{
  // TODO: error checking
  return d->edges[edge];
}

inline DATA_TYPE get_p(const struct gadmm_edge* e, const int i)
{
  // TODO: error checking
  return e->p[i];
}

inline DATA_TYPE get_u(const struct gadmm_edge* e, const int i)
{
  return e->u[i];
}

inline void set_p(struct gadmm_edge* e, const int i, const DATA_TYPE p)
{
  e->p[i] = p;
}

inline void set_u(struct gadmm_edge* e, const int i, const DATA_TYPE u)
{
  e->u[i] = u;
}

// get the number of edges out of vertex
inline int num_edges(const struct gadmm_vertex* d)
{
  return d->num_edges;
}

// get the length of the variable
inline int edge_length(const struct gadmm_edge *e)
{
  return e->len;
}

// XXX: implementation specific connect
void connect(struct gadmm_vertex *d1, const int e1, struct gadmm_vertex *d2, const int e2)
{
  free_edge(d2->edges[e2]);
  d2->edges[e2] = d1->edges[e1];
}

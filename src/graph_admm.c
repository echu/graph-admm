#include "graph_admm.h"
#include <stdlib.h>
#include <stdio.h>

typedef enum {RED, BLACK} vertex_color;

// an edge in the graph ADMM framework
struct gadmm_edge {
  int len;                // length of variable
  DATA_TYPE *p;           // variable
  DATA_TYPE *u;           // scaled dual variable
};

// a vertex in the graph ADMM framework
struct gadmm_vertex
{
  int num_edges;                // number of edges
  struct gadmm_edge **edges;    // my exposed edges
  vertex_color color;           // my vertex color
  
  // objective function pointer
  objective_func objective;
  // solver function pointer
  solver_func solve;
};

// returns a vertex with the functions set
struct gadmm_vertex *create_red_vertex(int len, int num_edges,
  objective_func f, solver_func prox_f)
{
  struct gadmm_vertex *new_vertex = (struct gadmm_vertex *) malloc(sizeof(struct gadmm_vertex));
  new_vertex->num_edges = num_edges;

  new_vertex->edges = (struct gadmm_edge **) malloc(num_edges*sizeof(struct gadmm_edge *));
  
  for(int i = 0; i < num_edges; i++)
    new_vertex->edges[i] = create_edge(len);
  
  new_vertex->color = RED;
  new_vertex->objective = f;
  new_vertex->solve = prox_f;
  
  return new_vertex;
}

struct gadmm_vertex *create_black_vertex(objective_func f, 
  solver_func prox_f)
{
  struct gadmm_vertex *new_vertex = (struct gadmm_vertex *) malloc(sizeof(struct gadmm_vertex));
  new_vertex->num_edges = 0;

  new_vertex->edges = NULL;
  
  new_vertex->color = BLACK;
  new_vertex->objective = f;
  new_vertex->solve = prox_f;
  
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
void free_red_vertex(struct gadmm_vertex *d)
{
  // check if the vertex is actually red
  if(d && d->color == RED) {
    if(d->edges) {
      for(int i = 0; i < d->num_edges; i++) 
        free_edge(d->edges[i]);
    }
    d->edges = NULL;
   
    free(d);
    d = NULL;
  }
}

// free black vertices (this implementation is specific!)
void free_black_vertex(struct gadmm_vertex *d)
{
  // check if the vertex is actually black
  if(d && d->color == BLACK) {
    if(d->edges) free(d->edges);
    d->edges = NULL;
   
    free(d);
    d = NULL;
  }
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
    e = NULL;
  }
}

// solve
void solve_vertex(struct gadmm_vertex *d, const DATA_TYPE rho, void *params)
{
  // TODO: only do error-checking when debugging
  if(d->solve) d->solve(d, rho, params);
}

// evaluate objective
DATA_TYPE evaluate_vertex(struct gadmm_vertex *d, void *params)
{
  if(d->objective)
    return d->objective(d, params);
  else
    return 0;
}

// accessors
const struct gadmm_edge* get_edge(struct gadmm_vertex *d, const int edge)
{
  // TODO: error checking
  return d->edges[edge];
}

struct gadmm_edge* get_mutable_edge(struct gadmm_vertex *d, const int edge)
{
  // TODO: error checking
  return d->edges[edge];
}

DATA_TYPE get_p(const struct gadmm_edge* e, const int i)
{
  // TODO: error checking
  return e->p[i];
}

DATA_TYPE get_u(const struct gadmm_edge* e, const int i)
{
  return e->u[i];
}

void set_p(struct gadmm_edge* e, const int i, const DATA_TYPE p)
{
  e->p[i] = p;
}

void set_u(struct gadmm_edge* e, const int i, const DATA_TYPE u)
{
  e->u[i] = u;
}

// get the number of edges out of vertex
int num_edges(const struct gadmm_vertex* d)
{
  return d->num_edges;
}

// get the length of the variable
int edge_length(const struct gadmm_edge *e)
{
  return e->len;
}

// XXX: implementation specific
void assign_edge(struct gadmm_vertex *from, const int e1, struct gadmm_vertex *to, const int e2)
{
  // allocate memory
  if(to->num_edges == 0) 
    to->edges = (struct gadmm_edge **) malloc(sizeof(struct gadmm_edge *));
  else
    to->edges = (struct gadmm_edge **) realloc(to->edges, (to->num_edges + 1)*sizeof(struct gadmm_edge *));

  to->edges[e2] = from->edges[e1];
  to->num_edges++;
}

// XXX: implementation specific connect
void connect(struct gadmm_vertex *d1, const int e1, struct gadmm_vertex *d2, const int e2)
{
  // only allow RED to connect to BLACK
  if(d1->color + d2->color == 1) {
    if(d1->color == BLACK) assign_edge(d2,e2, d1,e1);
    if(d2->color == BLACK) assign_edge(d1,e1, d2,e2); 
  } 
}



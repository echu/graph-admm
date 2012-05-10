#include "graph_admm.h"
#include <stdlib.h>

// a factor (net) in the graph ADMM framework
struct gadmm_factor
{
  // eventually, rho updates will go in here (i suppose)
  int num_connections;
  int connected;
  DATA_TYPE *pbar;
  struct gadmm_edge **neighbors;   // list of connections (to read / write on)
};

// an edge (terminal) in the graph ADMM framework
struct gadmm_edge {
  int T;                  // length of profiles
  DATA_TYPE *p;           // power profile
  DATA_TYPE *u;           // scaled price profile
  struct gadmm_factor *factor;   // factor
};

// a node (device) in the graph ADMM framework
struct gadmm_node
{
  int num_edges;                // number of edges
  struct gadmm_edge **edges;    // edges
  
  // objective function pointer
  DATA_TYPE (*objective)(struct gadmm_node*);
  // solver function pointer
  void (*solve)(struct gadmm_node*, const DATA_TYPE);
};


// returns a device with the functions set
struct gadmm_node *create_node(int T, int num_edges,
  DATA_TYPE (*objective_func)(struct gadmm_node*),
  void (*solver_func)(struct gadmm_node*, const DATA_TYPE))
{
  struct gadmm_node *new_node = malloc(sizeof(struct gadmm_node));
  new_node->num_edges = num_edges;

  new_node->edges = malloc(num_edges*sizeof(struct gadmm_edge *));
  
  for(int i = 0; i < num_edges; i++)
    new_node->edges[i] = create_edge(T);
  
  new_node->objective = objective_func;
  new_node->solve = solver_func;
  
  return new_node;
}

// returns a edge of length N with profiles and prices set to 0
struct gadmm_edge *create_edge(int len)
{
  struct gadmm_edge *new_edge = malloc(sizeof(struct gadmm_edge));
  new_edge->T = len;
  new_edge->p = calloc(len, sizeof(DATA_TYPE));
  new_edge->u = calloc(len, sizeof(DATA_TYPE));
  new_edge->factor = create_factor(len);
  return new_edge;
}

// creates a new factor (connections initialized to NULL)
struct gadmm_factor *create_factor(int len)
{
  struct gadmm_factor *new_factor = malloc(sizeof(struct gadmm_factor));
  new_factor->num_connections = 0;
  new_factor->connected = 0;
  new_factor->neighbors = NULL;
  new_factor->pbar = calloc(len, sizeof(DATA_TYPE));
  return new_factor;
}

// frees the memory
void free_node(struct gadmm_node *d)
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

void free_factor(struct gadmm_factor *f)
{
  if(f) {
    if(f->neighbors) free(f->neighbors);
    f->neighbors = NULL;
    if(f->pbar) free(f->pbar);
    f->pbar = NULL;
    free(f);
  }
  f = NULL;
}

/*
 * methods for the structs
 */
 
void add_neighbor(struct gadmm_factor *f, struct gadmm_edge *e)
{
  if(e != NULL) {
    if(f->num_connections == 0)
      f->neighbors = malloc(sizeof(struct gadmm_edge *));
    else 
      f->neighbors = realloc(f->neighbors, (f->num_connections+1)*sizeof(struct gadmm_edge *));
    
    f->neighbors[f->num_connections] = e;
    f->num_connections++;
  }
}

// XXX: IMPLEMENTATION SPECIFIC CODE
void send(struct gadmm_edge *e, DATA_TYPE *data)
{
  for(int i = 0; i < e->T; i++)
  {
    e->factor->pbar[i] += data[i];
  }
  
  e->factor->connected++;
}

void receive(struct gadmm_edge *e)
{
  // does nothing
}
// XXX


void update_node(struct gadmm_node *d)
{
  // send data to connections
  for(int i = 0; i < d->num_edges; i++) {
    struct gadmm_edge *e = d->edges[i];
    struct gadmm_factor *f = e->factor;
    
    if(f->connected == f->num_connections) {
      // u += pbar 
      // arg to device is p - pbar - u
      for(int i = 0; i < e->T; i++)
      {
        f->pbar[i] /= (f->num_connections+1);

        e->p[i] -= f->pbar[i];
        e->u[i] += f->pbar[i];

        f->pbar[i] = 0; // reset
      }
      f->connected = 0;
    } else { /* handle error */ }
  }
}


// solve
void solve_node(struct gadmm_node *d, const DATA_TYPE rho)
{
  d->solve(d, rho);
  
  // right after solving, we'll send things off
  for(int i = 0; i < d->num_edges; i++) {
    struct gadmm_edge *e = d->edges[i];
    struct gadmm_factor *f = e->factor;
    for(int j = 0; j < f->num_connections; j++) {
      // i need the user to provide this function
      send(f->neighbors[j], e->p);
    }
    
    // update your own factor
    for(int j = 0; j < e->T; j++) {
      f->pbar[j] += e->p[j];
    }
  }
  // then we enter the waiting state
}

inline DATA_TYPE evaluate_node(struct gadmm_node *d)
{
  return d->objective(d);
}


// update the edge (fixed rho in this version)
// u += pbar 
// arg to device is p - pbar - u
// inline void update_edge(struct gadmm_edge *e)
// {
//   for(int i = 0; i < e->T; i++)
//   {
//     e->p[i] -= e->factor->pbar[i];
//     e->u[i] += e->factor->pbar[i];
// 
//     e->factor->pbar[i] = 0; // reset
//   }
// }

inline struct gadmm_factor* get_factor(struct gadmm_node* d, const int edge)
{
  return d->edges[edge]->factor;
}

inline struct gadmm_edge* get_edge(struct gadmm_node *d, const int edge)
{
  // TODO: error checking
  return d->edges[edge];
}

inline DATA_TYPE get_p(const struct gadmm_node* d, const int edge, const int t)
{
  // TODO: error checking
  return d->edges[edge]->p[t];
}

inline DATA_TYPE get_u(const struct gadmm_node* d, const int edge, const int t)
{
  return d->edges[edge]->u[t];
}

inline void set_p(struct gadmm_node* d, const int edge, const int t, const DATA_TYPE p)
{
  d->edges[edge]->p[t] = p;
}

inline void set_u(struct gadmm_node* d, const int edge, const int t, const DATA_TYPE u)
{
  d->edges[edge]->u[t] = u;
}


// get the number of edges out of node
inline int num_edges(const struct gadmm_node* d)
{
  return d->num_edges;
}

// get the length of the variable
inline int edge_length(const struct gadmm_node *d, const int i)
{
  return d->edges[i]->T;
}


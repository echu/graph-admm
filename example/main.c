#include "graph_admm.h"
#include <stdio.h>

static float alpha = 0.01;
static float beta = 0.1;

static float load[] = { 1.1, 1.0, 0.9, 0.8, 1.2, 1.4, 1.5, 1.8, 1.9, 2.0 };


/* ------------------- *
 * OBJECTIVE FUNCTIONS *
 * ------------------- */
 
// the objective on a load
float load_objective(struct gadmm_vertex *d, void *ignore)
{
  return 0.0;
}

// the objective for a generator
float gen_objective(struct gadmm_vertex *d, void *ignore)
{
  float obj = 0;
  const struct gadmm_edge *e  = get_edge(d,0);
  for(int i = 0; i < edge_length(e); i++)
    obj += alpha*get_p(e, i)*get_p(e, i) - beta*get_p(e, i);
  return obj;
}

/* ---------------- *
 * SOLVER FUNCTIONS *
 * ---------------- *
 * These currently use the simplified version of the MSG_PASS_DYN
 * algorithm. If needed, they could be adjusted to use the more 
 * generic ADMM algorithm. 
 */

// the solver for a load
void load_solve(struct gadmm_vertex *s, const float rho, void *l)
{
  float *myload = (float *) l;
  for(int i = 0; i < num_edges(s); i++)
  {
    struct gadmm_edge *e  = get_mutable_edge(s,i);
    for(int j = 0; j < edge_length(e); j++)
      set_p(e, j, myload[j]); // solve for this terminal
  }
}

// the solver for a generator
void gen_solve(struct gadmm_vertex *s, const float rho, void *genparams)
{
  for(int i = 0; i < num_edges(s); i++)
  {
    struct gadmm_edge *e  = get_mutable_edge(s,i);
    for(int j = 0; j < edge_length(e); j++) {
      float v = get_p(e, j) - get_u(e, j);
      float tmp = (rho*v + beta)/(2*alpha + rho);
      tmp = (tmp >= 0) ? 0 : tmp;
      tmp = (tmp <= -3.0) ? -3.0 : tmp;
      set_p(e, j, tmp);
    }
  }
}

// the solver for a lossless line
void line_solve(struct gadmm_vertex *s, const float rho, void *lineparams)
{
  float tmp[10];
  for(int i = 0; i < num_edges(s); i++) {
    const struct gadmm_edge *e  = get_edge(s,i);
    for(int j = 0; j < edge_length(e); j++) {
      if(i == 0) tmp[j] = 0;
      tmp[j] += get_p(e, j) - get_u(e, j);
    }
  }
  
  // send data back
  for(int i = 0; i < num_edges(s); i++) {
    struct gadmm_edge *e  = get_mutable_edge(s,i);
    for(int j = 0; j < edge_length(e); j++) {
      set_p(e, j, get_p(e, j) - get_u(e, j) - tmp[j]/num_edges(s));
    }
  }
}

// the solver for a bus
void physics(struct gadmm_vertex *s, const float rho, void *physicsparam)
{
  float tmp[10];
  for(int i = 0; i < num_edges(s); i++) {
    const struct gadmm_edge *e  = get_edge(s,i);
    for(int j = 0; j < edge_length(e); j++) {
      if(i == 0) tmp[j] = 0;
      tmp[j] += get_p(e, j);
    }
  }
  
  // send the data back
  for(int i = 0; i < num_edges(s); i++) {
    struct gadmm_edge *e  = get_mutable_edge(s,i);
    for(int j = 0; j < edge_length(e); j++) {
      set_p(e, j, get_p(e,j) - tmp[j]/num_edges(s));
      set_u(e, j, get_u(e,j) + tmp[j]/num_edges(s));
    }
  }
}

/* ------------- *
 * MAIN FUNCTION *
 * ------------- */
int main(int argc, char **argv)
{
  // red vertices
  struct gadmm_vertex *l = create_vertex(10, 1, &load_objective, &load_solve);
  struct gadmm_vertex *g = create_vertex(10, 1, &gen_objective, &gen_solve);
  struct gadmm_vertex *line = create_vertex(10, 2, &load_objective, &line_solve);
  
  // black vertices
  struct gadmm_vertex *bus1 = create_vertex(0,0, &load_objective, &physics);
  struct gadmm_vertex *bus2 = create_vertex(0,0, &load_objective, &physics);
  

  // connect the load and the line to a bus
  connect(l,0, bus1,0);
  connect(line,0, bus1,1);
  
  // connect the generator and the other end of the line to a bus
  connect(g,0, bus2,0);
  connect(line,1, bus2,1);
  
  // XXX: need some sort of asynchronous mechanism
  for(int iter = 0; iter < 1000; iter++) {
    // SOLVE red
    solve_vertex(l, 1.0, load);
    solve_vertex(g, 1.0, NULL);
    solve_vertex(line, 1.0, NULL);
    
    // SOLVE black
    solve_vertex(bus1, 1.0, NULL);
    solve_vertex(bus2, 1.0, NULL);
  }
  
  for(int i = 0; i < 10; i++)
    printf("%f\n", get_p(get_edge(l,0),i));
  
  for(int i = 0; i < 10; i++)
    printf("%f\n", get_p(get_edge(g,0),i));
  
  
  printf("~~~ %f ~~~\n", evaluate_vertex(g, NULL));
  
  // free red nodes
  free_vertex(g);
  free_vertex(l);
  free_vertex(line);
  
  // don't need to free black nodes
  //free_vertex(bus1);
  //free_vertex(bus2);

}

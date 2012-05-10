#include "graph_admm.h"
#include <stdio.h>

static float alpha = 0.01;
static float beta = 0.1;

static float load[] = { 1.1, 1.0, 0.9, 0.8, 1.2, 1.4, 1.5, 1.8, 1.9, 2.0 };


// objectives
float load_objective(struct gadmm_node *d)
{
  return 0.0;
}

float gen_objective(struct gadmm_node *d)
{
  float obj = 0;
  for(int i = 0; i < edge_length(d, 0); i++)
  {
    obj += alpha*get_p(d, 0, i)*get_p(d, 0, i) - beta*get_p(d, 0, i);
  }
  return obj;
}

// solvers
void load_solve(struct gadmm_node *s, const float rho)
{
  for(int i = 0; i < num_edges(s); i++)
  {
    for(int j = 0; j < edge_length(s,i); j++)
      set_p(s, i, j, load[j]); // solve for this terminal
  }
}

void gen_solve(struct gadmm_node *s, const float rho)
{
  for(int i = 0; i < num_edges(s); i++)
  {
    for(int j = 0; j < edge_length(s,i); j++) {
      float v = get_p(s, i, j) - get_u(s, i, j);
      float tmp = (rho*v + beta)/(2*alpha + rho);
      tmp = (tmp >= 0) ? 0 : tmp;
      tmp = (tmp <= -3.0) ? -3.0 : tmp;
      set_p(s, i, j, tmp);
    }
  }
}

void line_solve(struct gadmm_node *s, const float rho)
{
  float tmp[10];
  for(int i = 0; i < num_edges(s); i++) {
    for(int j = 0; j < edge_length(s,i); j++) {
      if(i == 0) tmp[j] = 0;
      tmp[j] += get_p(s, i, j) - get_u(s, i, j);
    }
  }
  for(int i = 0; i < num_edges(s); i++) {
    for(int j = 0; j < edge_length(s,i); j++) {
      set_p(s, i, j, get_p(s, i, j) - get_u(s, i, j) - tmp[j]/num_edges(s));
    }
  }
}

// you should specify how the edges are connected
void connect(struct gadmm_node *d1, const int e1, struct gadmm_node *d2, const int e2)
{
  add_neighbor(get_factor(d1, e1), get_edge(d2, e2));
  add_neighbor(get_factor(d2, e2), get_edge(d1, e1));
}


int main(int argc, char **argv)
{
  struct gadmm_node *l = create_node(10, 1, &load_objective, &load_solve);
  struct gadmm_node *g = create_node(10, 1, &gen_objective, &gen_solve);
  struct gadmm_node *line = create_node(10, 2, &load_objective, &line_solve);
  
  // problems can happen here if you subscribe using an *incompatible* proj_func
  //    you might crash if published message is unexpected
  //    error-checking can be done in proj_func
  //    error-handling can also be done in proj_func
  //
  // we leave it completely up to the user to ensure that compatible proj_funcs
  // are being used when subscribing
  //
  //subscribe(l,'abc', proj_func);
  //subscribe(g,'abc', proj_func);
  //subscribe(l,'abc', proj_func);  // do nothing?
  
  connect(l,0, line,0);
  connect(g,0, line,1);
  
  
  // XXX: make the following a finite state machine?
  // XXX: need some sort of asynchronous receive
  for(int iter = 0; iter < 1000; iter++) {
    
    // SOLVE state
    solve_node(l, 1.0);
    solve_node(g, 1.0);
    solve_node(line, 1.0);
    
    // BROADCAST immediately after
    
    // WAIT / RECEIVE state
    
    // each node enters wait state
    // 
    update_node(l);
    update_node(g);
    update_node(line);
    //
    // update_node broadcasts "p" to its terminals' neighbors
    // then waits for num_neighbors msgs to return
  
  }
  
  
  for(int i = 0; i < 10; i++)
  {
    printf("%f\n", get_p(l,0,i));
  }
  
  for(int i = 0; i < 10; i++)
  {
    printf("%f\n", get_p(g,0,i));
  }
  
  printf("~~~ %f ~~~\n", evaluate_node(g));
  
  free_node(g);
  free_node(l);

}

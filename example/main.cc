#include "graph_admm.h"
#include <vector>

using namespace graph_admm;

class Vector : public AbelianGroupInterface
{
  public:
    Vector() : data(NULL) { };
    
    ~Vector() { Reset(); }
    
    void Initialize()
    {
      if(!data) data = new double[n];
    }
  
    void AddInPlace(const AbelianGroupInterface& rhs)
    {
      const Vector &vec_rhs = static_cast<const Vector&>(rhs);
      for(int i = 0; i < n; i++) {
        this->data[i] += vec_rhs.data[i];
      }
    }

    void SubInPlace(const AbelianGroupInterface& rhs) 
    {
      const Vector &vec_rhs = static_cast<const Vector&>(rhs);
      for(int i = 0; i < n; i++) {
        this->data[i] -= vec_rhs.data[i];
      }
    }
    
    void Reset()
    {
      if(data) delete[] data;
      data = NULL;
    }

    static Vector* Zero()
    {
      Vector *vec = new Vector;
      vec->Initialize();
      return vec;
    }
  
  private:
    static const int n = 10;
    double *data;
};

class Generator : public Vertex {
  public:
    Generator(const char *s) : Vertex(s) { }

    void Prox() 
    { 
    
    }
};

class Load : public Vertex {
  public:
    Load(const char *s) : Vertex(s) { }
    
    void Prox()
    {
      
    }
};

int main(int argc, char **argv)
{
  std::vector<Vertex*> vertices;
  Generator v1("palo alto");
  Generator v2("justice");
  Load l1("stanford");
  
  std::vector<const char *> names(2);
  names[0] = "v1.in"; names[1] = "v2.in";
  v1.InitRedVertex<Vector>(names);
  
  vertices.push_back(&v1);
  vertices.push_back(&v2);
  vertices.push_back(&l1);
  
  std::vector<Vertex*>::const_iterator it;
  for ( it = vertices.begin();  it != vertices.end(); ++it )
    printf("%s\n", (*it)->name());
}

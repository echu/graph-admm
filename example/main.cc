#include "graph_admm.h"
#include <vector>

using namespace graph_admm;

class Vector : public RingInterface
{
  public:
    Vector() : data(NULL) { };
    
    ~Vector() { Reset(); }
    
    void Initialize()
    {
      if(!data) data = new double[n];
    }
  
    virtual void AddInPlace(const RingInterface& y)
    {
      if(data) {
        const Vector &rhs = static_cast<const Vector&>(y);
        for(int i = 0; i < n; i++) {
          this->data[i] += rhs.data[i];
        }
      }
    }

    virtual void SubInPlace(const RingInterface& y) 
    {
      if(data) {
        const Vector &rhs = static_cast<const Vector&>(y);
        for(int i = 0; i < n; i++) {
          this->data[i] -= rhs.data[i];
        }
      }
    }
    
    virtual void ScaleInPlace(const double &a)
    {
      if(data) {
        for(int i = 0; i < n; i++) {
          this->data[i] *= a;
        }
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
    
    double& operator[](const int& k)
    {
      return data[k];
    }
    
    const int length() const
    {
      return n;
    }
  
  private:
    static const int n = 10;
    double *data;
};

class Generator : public Vertex {
  public:
    Generator(const char *s) : Vertex(s) { 
      std::vector<const char *> names(1);
      names[0] = "out";
      this->InitRedVertex<Vector>(names);
    }
  
    virtual void Prox(Const_ProxArg& in, ProxArg& out) 
    { 
    
    }
};

class Load : public Vertex {
  public:
    Load(const char *s) : Vertex(s) { 
      std::vector<const char *> names(2);
      names[0] = "out";
      this->InitRedVertex<Vector>(names);
    }

    virtual void Prox(Const_ProxArg& in, ProxArg& out) 
    {
      for(int i = 0; i < 10; i++)
      {
        out["out"][i] = in["out"][i];
      }
    }
};

int main(int argc, char **argv)
{
  std::vector<Vertex*> vertices;
  Generator v1("palo alto");
  Generator v2("justice");
  Load l1("stanford");

  vertices.push_back(&v1);
  vertices.push_back(&v2);
  vertices.push_back(&l1);
  
  std::vector<Vertex*>::const_iterator it;
  for ( it = vertices.begin();  it != vertices.end(); ++it )
    printf("%s\n", (*it)->name());
}

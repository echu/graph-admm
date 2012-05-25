#ifndef GRAPH_ADMM_GRAPH_ADMM_H_
#define GRAPH_ADMM_GRAPH_ADMM_H_

#include <sparsehash/dense_hash_map>
#include <vector>

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
// (ECHU): From Google's C++ styleguide
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

namespace graph_admm {

// An Abelian group interface (AbelianGroupInterface)
// ==================================================
// This interface simply requires that "terminal" data structures be an
// Abelian (commutative) group. This means that objects which inherit 
// the "AbelianGroupInterface" have the "+" operation defined and an 
// inverse element (or, similarly, the "-" operation defined). 
// Furthermore, these elements are in the group (i.e., also objects of 
// type AbelianGroupInterface).
//
// The user must also specify the identity element.
//
// For a concrete implementation of this interface, the "+" and "-" 
// operations are specified in-place.
//
// Instead of operator overloading, we force the user to implement the 
// functions so that they are conscious of the design choices. This is 
// also in accordance with the Google styleguide.
//
class AbelianGroupInterface {
  public:
    virtual ~AbelianGroupInterface() { };
    // Virtual method for adding in place
    // ==================================
    // The mathematical semantics of this funcion call is
    //
    //    x += y
    //
    // Since C++ doesn't support covariant arguments, we guarantee that
    // the argument to AddInPlace will be of the derived class for this
    // interface.
    virtual void AddInPlace(const AbelianGroupInterface& rhs) = 0;
    
    // Virtual method for subtracting in place
    // ========================================
    // The mathematical semantics of this funcion call is
    //
    //    x += (-y)
    //
    // Since C++ doesn't support covariant arguments, we guarantee that
    // the argument to SubInPlace will be of the derived class for this
    // interface.
    virtual void SubInPlace(const AbelianGroupInterface& rhs) = 0;
    
    // Virtual method for deleting element
    // ===================================
    virtual void Reset() = 0;
    
    // Factory method for generating identity element
    // ==============================================
    // This is a static member function to create the identity element.
    static AbelianGroupInterface* Zero();
};


// An ADMM vertex class (Vertex)
// =============================
// This base class implements behavior for any ADMM vertex. It requires 
// subclasses to implement the prox operator and, optionally, the 
// objective function.
//
// blah blah blah
class Vertex {
  public:
    explicit Vertex(const char *s) : name_(s), color(kNone) 
    { 
      terminals.set_empty_key(NULL);
    }
    
    virtual const double EvaluateObjective() const { return 0.0; }
    virtual void Prox() = 0;
    
    virtual ~Vertex() { Reset(); } 
    // other things to implement later...
    
    // get name
    const char *name() const { return name_; }
      
    void Reset();
    
  protected:
    // initializers
    template<typename T>
      void InitRedVertex(const std::vector<const char *> &term_names);
    template<typename T>
      void InitBlackVertex(const std::vector<const char *> &term_names);
    
  private:
    DISALLOW_COPY_AND_ASSIGN(Vertex);
    
    // enum for VertexColors
    enum VertexColors {
      kNone = 0,
      kRed,
      kBlack
    };
    
    // An ADMM terminal struct (Terminal)
    // =================================
    // This struct is a container for any ADMM terminal. It requires the 
    // data types to be subclasses of an Abelian group.
    //
    // We use scoped_ptr's instead of a regular pointer to ensure that
    // the memory is properly managed.
    struct Terminal {
      AbelianGroupInterface *x;
      AbelianGroupInterface *z;
      AbelianGroupInterface *u;
      double rho;
    };
    
    // functor for Dan Bernstein hash function
    struct djb2 {
      size_t operator()(const char *str) const;
    };
    
    // functor for string comparison
    struct eqstr {
      bool operator()(const char *s1, const char* s2) const
      {
        return (s1 == s2) || (s1 && s2 && strcmp(s1,s2) == 0);
      }
    };
    
    const char *name_;
    google::dense_hash_map<const char *, Terminal, djb2, eqstr> terminals;
    VertexColors color;
};



// red vertex, black vertex inherit from vertex
// vertex has a private class which is a terminal

// specify terminal names and (number of terminals) on init
// init_as_red, init_as_black

// contains stringval hash for terminals
// name, color, solve, obj
// TYPE for x,z,u

size_t Vertex::djb2::operator()(const char *str) const
{
  size_t hash = 5381;
  size_t c;

  while ((c = *str++) != '\0')
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

template <typename T>
void Vertex::InitRedVertex(const std::vector<const char *> &term_names) 
{
  color = kRed;
  for ( 
    std::vector<const char *>::const_iterator it = term_names.begin(); 
    it != term_names.end();
    ++it ) {
      Terminal term;
     
      term.x = T::Zero();
      term.z = T::Zero();
      term.u = T::Zero();
      term.rho = 1.0;
      
      this->terminals[*it] = term;
  }
}

template <typename T>
void Vertex::InitBlackVertex(const std::vector<const char *> &term_names) 
{
  color = kBlack;
  for ( 
    std::vector<const char *>::const_iterator it = term_names.begin(); 
    it != term_names.end();
    ++it ) {
      Terminal term;
      
      term.x = T::Zero();
      term.z = T::Zero();
      term.u = T::Zero();
      term.rho = 1.0;
      
      this->terminals[*it] = term;
  }
}

void Vertex::Reset() 
{
  google::dense_hash_map<const char *, Terminal, djb2, eqstr>
    ::const_iterator it;
  
  for(it = terminals.begin(); it != terminals.end(); ++it) {
    it->second.x->Reset();
    it->second.z->Reset();
    it->second.u->Reset();
  }
}

} // namespace graph_admm

#endif // GRAPH_ADMM_GRAPH_ADMM_H_
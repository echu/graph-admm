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

// A ring interface (RingInterface)
// ================================
// This interface simply requires that "terminal" data structures be a
// ring. This means that objects which inherit the "RingInterface" have 
// the "+" operation defined and an (additive) inverse element (or, 
// similarly, the "-" operation defined). Furthermore, if x and y are 
// concrete objects of the RingInterface, then x + y and x - y are also 
// in the ring (i.e., also objects of type RingInterface).
//
// The user must also specify the identity element (i.e., "0").
//
// Since this is a ring interface, it must also define a "*" operation.
// However, "*" is limited in the sense that we only consider a*x, where
// a is a real number and x is an element in the ring. (Hence, it's not
// actually a ring in the mathematical sense.)
//
// For a concrete implementation of this interface, the "+" and "-" 
// operations are specified in-place. The scalar multiply is also 
// in-place. The identity element (or zero element) is a factory method
// that produces a concrete element of this type.
//
// Instead of operator overloading, we force the user to implement the 
// functions so that they are conscious of the design choices. This is 
// also in accordance with the Google styleguide.
//
class RingInterface {
  public:
    virtual ~RingInterface() { };
    // Virtual method for adding in place
    // ==================================
    // The mathematical semantics of this funcion call is
    //
    //    x += y
    //
    // Since C++ doesn't support covariant arguments, we guarantee that
    // the argument to AddInPlace will be of the derived class for this
    // interface.
    virtual void AddInPlace(const RingInterface& y) = 0;
    
    // Virtual method for subtracting in place
    // ========================================
    // The mathematical semantics of this funcion call is
    //
    //    x += (-y)
    //
    // Since C++ doesn't support covariant arguments, we guarantee that
    // the argument to SubInPlace will be of the derived class for this
    // interface.
    virtual void SubInPlace(const RingInterface& y) = 0;
    
    // Virtual method for scalar multiply in place
    // ===========================================
    // The mathematical semantics of this funcion call is
    //
    //    x *= a,
    //
    // where "a" is a scalar (a real number).
    virtual void ScaleInPlace(const double& a) = 0;
    
    // Factory method for generating identity element
    // ==============================================
    // This is a static member function to create the identity element.
    static RingInterface* Zero();
};

// A connection interface (ConnectionInterface)
// ============================================
// Objects which implement this interface are able to exchange messages
// with other objects implementing this interface. This sits on the
// application layer in the network stack and wraps the lower-level
// transport layers. For instance, this interface could be implemented
// via sockets or ZeroMQ or a Redis database. 
//
// The interface specifies how the connection is initialized (is it a 
// server? or a client?), and what happens when a RingInterface object 
// is sent and received. As argument, the Init function receives a 
// string specifying the (global) name of the terminal. 
// 
// The interface does not specify how two objects connect (this is done 
// through specifying the Init function), but rather that they are able 
// to send messages of type RingInterface and also decode messages of 
// that type.
class ConnectionInterface {
  public:
    virtual ~ConnectionInterface() { };
  
    virtual void Init(const char *arg) = 0;
    virtual void Send(const RingInterface *msg) = 0;
    virtual RingInterface *Recv() = 0;
};


// An ADMM vertex class (Vertex)
// =============================
// This base class implements behavior for any ADMM vertex. It requires 
// subclasses to implement the prox operator and, optionally, the 
// objective function.
//
// The Vertex class exposes a public type called ProxArg, which is a
// dictionary of pointers to any derived classes of the RingInterface. 
// It can be used externally as follows:
//
// Vertex::ProxArg a;
// a["hello"] = v;
//
// where v is an object derived from RingInterface.
//
// The choice to expose this is because ProxArg is already exposed to
// subclasses in the virtual Prox function, which could be made public
// by subclasses.
//
// The private ProxArg's "inputs" and "outputs" maintain pointers to
// RingInterfaces. They point directly to the RingInterfaces stored
// inside a Terminal. This way, modifications to Terminal propagate
// to the prox inputs and modifications to the prox outputs propagate
// to the Terminals.
//
// Blacks act as servers...
class Vertex {
  public:
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

    // hash map for pointers to prox arguments
    typedef google::dense_hash_map<const char *, const RingInterface *, djb2, eqstr> Const_ProxArg;
    typedef google::dense_hash_map<const char *, RingInterface *, djb2, eqstr> ProxArg;

    // constructor
    explicit Vertex(const char *s) : name_(s), color_(kNone) 
    { 
      terminals_.set_empty_key(NULL);
      inputs.set_empty_key(NULL);
      outputs.set_empty_key(NULL);
    }
    
    // functions for Derived classes to implement
    virtual inline const double EvaluateObjective() const { return 0.0; }
    virtual void Prox(Const_ProxArg &in, ProxArg &out) = 0;
    
    virtual ~Vertex() { Reset(); } 
    
    // get name
    inline const char *name() const { return name_; }
    // get terminal values
    inline const RingInterface *terminal_x(const char *str) 
      { return terminals_[str].x; }
    inline const RingInterface *terminal_z(const char *str) 
      { return terminals_[str].z; }
    inline const RingInterface *terminal_u(const char *str) 
      { return terminals_[str].u; }
    
    // not a deep copy
    void set_terminal_x(const char *str, RingInterface *r)
      { terminals_[str].x = r; }
    void set_terminal_z(const char *str, RingInterface *r)
      { terminals_[str].z = r; }
    void set_terminal_u(const char *str, RingInterface *r)
      { terminals_[str].u = r; }
      
    // to clear memory
    void Reset();
    
    // does the appropriate ADMM update
    void Update();
    
  protected:
    // initializers
    template<typename T, typename U>
      void InitRedVertex(const std::vector<const char *> &term_names);
    template<typename T, typename U>
      void InitBlackVertex(const std::vector<const char *> &term_names);
    
  private:    
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
    // We (the graph_admm header library) are responsible for freeing
    // the concrete ring objects for x, z, and u.
    struct Terminal {
      ConnectionInterface *connection;
      RingInterface *x;
      RingInterface *z;
      RingInterface *u;
      double rho;
    };
    
    DISALLOW_COPY_AND_ASSIGN(Vertex);
    
    // name of vertex
    const char *name_;
    // hash map for terminals
    typedef google::dense_hash_map<const char *, Terminal, djb2, eqstr> TerminalDict;
    TerminalDict terminals_;
    
    // hash map for arguments of prox function
    Const_ProxArg inputs;
    ProxArg outputs;
    
    // color of vertex
    VertexColors color_;
};

size_t Vertex::djb2::operator()(const char *str) const
{
  size_t hash = 5381;
  size_t c;

  while ((c = *str++) != '\0')
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

template <typename T, typename U>
void Vertex::InitRedVertex(const std::vector<const char *> &term_names) 
{
  color_ = kRed;
  for ( 
    std::vector<const char *>::const_iterator it = term_names.begin(); 
    it != term_names.end();
    ++it ) {
      Terminal term;
     
      term.x = T::Zero();
      term.z = T::Zero();
      term.u = T::Zero();
      term.rho = 1.0;
      term.connection = new U;
      term.connection->Init(*it);
      
      this->terminals_[*it] = term;
      this->inputs[*it] = term.z;   // term.z is container of prox input
      this->outputs[*it] = term.x;  // term.x is prox output
  }
}

template <typename T, typename U>
void Vertex::InitBlackVertex(const std::vector<const char *> &term_names) 
{
  color_ = kBlack;
  for ( 
    std::vector<const char *>::const_iterator it = term_names.begin(); 
    it != term_names.end();
    ++it ) {
      Terminal term;
      
      term.x = T::Zero();
      term.z = T::Zero();
      term.u = T::Zero();
      term.rho = 1.0;
      term.connection = new U;
      term.connection->Init(*it);
      
      this->terminals_[*it] = term;
      this->inputs[*it] = term.x;   // term.x is container of prox input
      this->outputs[*it] = term.z;  // term.z is prox output
  }
}

void Vertex::Reset() 
{
  TerminalDict::iterator it;
  
  for(it = terminals_.begin(); it != terminals_.end(); ++it) {
    delete it->second.x;
    delete it->second.z;
    delete it->second.u;
    delete it->second.connection;
    it->second.x = NULL;
    it->second.z = NULL;
    it->second.u = NULL;
    it->second.connection = NULL;
  }
}

void Vertex::Update()
{
  // Update() is a thread critical block!
  TerminalDict::iterator it;
  if (color_ == kRed) {
    for(it = terminals_.begin(); it != terminals_.end(); ++it) {
      Terminal term = it->second;
      // receive z
      term.z = term.connection->Recv();
      // dual variable update (assumes z's been updated)
      term.x->SubInPlace(*term.z);
      term.u->AddInPlace(*term.x);
      
      // pass in z - u
      term.z->SubInPlace(*term.u);
    }
    
    // inputs and outputs point to the proper terminal args
    Prox(inputs, outputs);
    
    // send term.x 
    for(it = terminals_.begin(); it != terminals_.end(); ++it) {
      Terminal term = it->second;
      term.connection->Send(term.x);
    }
    
  } else if (color_ == kBlack) {
    // pass in x + u, (assumes x's been updated)
    for(it = terminals_.begin(); it != terminals_.end(); ++it) {
      Terminal term = it->second;
      // receive x
      term.x = term.connection->Recv();
      term.x->AddInPlace(*term.u);
    }
    
    Prox(inputs, outputs);// term.z = Prox(term.x);
    
    // dual variable update
    for(it = terminals_.begin(); it != terminals_.end(); ++it) {
      Terminal term = it->second;
      term.u->SubInPlace(*term.z);
      // send z
      term.connection->Send(term.z);
    }
    // send term.z
  }
  // else do nothing
}

} // namespace graph_admm

#endif // GRAPH_ADMM_GRAPH_ADMM_H_
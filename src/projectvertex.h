// (c) Robert Kloefkorn 2004 - 2010 
#ifndef ALUGRIDVERTEXPROJECTION_H_INCLUDED
#define ALUGRIDVERTEXPROJECTION_H_INCLUDED

namespace ALUGridSpace {

// use standard namespace 
using namespace std;

// interface class for projecting vertices for boundary adjustment 
template <int dim> 
class VertexProjection
{
protected:
  // don't allow creation of an instance  
  VertexProjection () {}
public:
  // destructor 
  virtual ~VertexProjection() {}
  // projection method 
  virtual int operator()(const double (&p)[dim],
                         const int segmentIndex,
                         double (&ret)[dim]) const = 0;
};

} // end namespace 
#endif

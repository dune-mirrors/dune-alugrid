// (c) Robert Kloefkorn 2004 - 2010 
#ifndef ALUGRIDVERTEXPROJECTION_H_INCLUDED
#define ALUGRIDVERTEXPROJECTION_H_INCLUDED

#define ALUGRID_VERTEX_PROJECTION

// interface class for projecting vertices for boundary adjustment 
template <int dim, class coord_t = double > 
class VertexProjection
{
protected:
  // don't allow creation of an instance  
  VertexProjection () {}
public:
  // destructor 
  virtual ~VertexProjection() {}
  // projection method 
  virtual int operator()(const coord_t (&p)[dim],
                         const int segmentIndex,
                         coord_t (&ret)[dim]) const = 0;
};

#endif // #ifndef ALUGRIDVERTEXPROJECTION_H_INCLUDED

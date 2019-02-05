// (c) Robert Kloefkorn 2004 - 2010
#ifndef ALUGRIDVERTEXPROJECTION_H_INCLUDED
#define ALUGRIDVERTEXPROJECTION_H_INCLUDED

namespace ALUGrid
{
  // interface class for projecting vertices for boundary adjustment
  template <class Buffer, int dim, class coord_t = double >
  class VertexProjection
  {
    typedef VertexProjection< ObjectStream, dim, coord_t > ThisType;
  protected:
    // don't allow creation of an instance
    VertexProjection () {}

  public:
    typedef ThisType* factory_t( Buffer& buffer );
    typedef factory_t*  factoryptr_t ;

    static factoryptr_t& factory()
    {
      static factoryptr_t f = nullptr;
      return f;
    }

    static void setFactoryMethod( factory_t * f )
    {
      factory() = f ;
    }

    typedef Buffer BufferType;

    // destructor
    virtual ~VertexProjection() {}
    // projection method
    virtual int operator()(const coord_t (&p)[dim],
                           const int segmentId,
                           coord_t (&ret)[dim]) const = 0;

    // projection method
    virtual int operator()(const coord_t (&p)[dim],
                           coord_t (&ret)[dim]) const = 0;

    virtual void backup( Buffer& ) const { std::abort(); }

    virtual bool valid () const { return false; }

    static ThisType* restore( Buffer& buffer )
    {
      alugrid_assert( factory() );
      return factory()( buffer );
    }
  };

} // namespace ALUGrid

#endif // #ifndef ALUGRIDVERTEXPROJECTION_H_INCLUDED

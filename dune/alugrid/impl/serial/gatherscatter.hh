#ifndef ALUGRID_SRC_SERIAL_GATHERSCATTER_HH
#define ALUGRID_SRC_SERIAL_GATHERSCATTER_HH

#include "gitter_sti.h"
#include "serialize.h"

namespace ALUGrid
{

  // GatherScatter
  // -------------

  struct GatherScatter
  {
    // type of used object stream 
    typedef ObjectStreamImpl ObjectStreamType;

    virtual ~GatherScatter () {}

    // return true if user defined partitioning methods should be used 
    virtual bool userDefinedPartitioning () const { alugrid_assert (false); abort(); return false ; }
    // return true if user defined load balancing weights are provided
    virtual bool userDefinedLoadWeights () const { alugrid_assert (false); abort(); return false ; }
    // returns true if user defined partitioning needs to be readjusted 
    virtual bool repartition () { alugrid_assert (false); abort(); return false; }

    // return load weight of given element 
    virtual int loadWeight( const Gitter::helement_STI &elem ) const { alugrid_assert (false); abort(); return 1; }

    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    virtual int destination( const Gitter::helement_STI &elem ) const { alugrid_assert (false); abort(); return -1; }

    virtual bool contains(int,int) const = 0;

    virtual bool containsItem(const Gitter::helement_STI &elem ) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsItem(const Gitter::hface_STI   & elem ) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsItem(const Gitter::hedge_STI   & elem ) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsItem(const Gitter::vertex_STI & elem ) const { alugrid_assert (false); abort(); return false; }
    
    virtual bool containsInterior (const Gitter::hface_STI  & face , ElementPllXIF_t & elif) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsGhost    (const Gitter::hface_STI  & face , ElementPllXIF_t & elif) const { alugrid_assert (false); abort(); return false; }
    
    virtual void inlineData ( ObjectStreamType & str , Gitter::helement_STI & elem ) { alugrid_assert (false); abort(); }
    virtual void xtractData ( ObjectStreamType & str , Gitter::helement_STI & elem ) { alugrid_assert (false); abort(); }
    
    virtual void sendData ( ObjectStreamType & str , Gitter::hface_STI & elem ) { alugrid_assert (false); abort(); }
    virtual void recvData ( ObjectStreamType & str , Gitter::hface_STI & elem ) { alugrid_assert (false); abort(); }
    virtual void setData  ( ObjectStreamType & str , Gitter::hface_STI & elem ) { alugrid_assert (false); abort(); }
    
    virtual void sendData ( ObjectStreamType & str , Gitter::hedge_STI & elem ) { alugrid_assert (false); abort(); }
    virtual void recvData ( ObjectStreamType & str , Gitter::hedge_STI & elem ) { alugrid_assert (false); abort(); }
    virtual void setData  ( ObjectStreamType & str , Gitter::hedge_STI & elem ) { alugrid_assert (false); abort(); }
    
    virtual void sendData ( ObjectStreamType & str , Gitter::vertex_STI & elem ) { alugrid_assert (false); abort(); }
    virtual void recvData ( ObjectStreamType & str , Gitter::vertex_STI & elem ) { alugrid_assert (false); abort(); }
    virtual void setData  ( ObjectStreamType & str , Gitter::vertex_STI & elem ) { alugrid_assert (false); abort(); }

    virtual void sendData ( ObjectStreamType & str , const Gitter::helement_STI  & elem ) { alugrid_assert (false); abort(); }
    virtual void sendData ( ObjectStreamType & str , const Gitter::hbndseg & elem ) { alugrid_assert (false); abort(); }
    virtual void recvData ( ObjectStreamType & str , Gitter::hbndseg & elem ) { alugrid_assert (false); abort(); }
    virtual void recvData ( ObjectStreamType & str , Gitter::helement_STI  & elem ) { alugrid_assert (false); abort(); }
  };

  typedef GatherScatter GatherScatterType;

} // namespace ALUGrid

#endif // #ifndef ALUGRID_SRC_SERIAL_GATHERSCATTER_HH

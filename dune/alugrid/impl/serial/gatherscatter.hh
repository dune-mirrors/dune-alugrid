#ifndef ALUGRID_SRC_SERIAL_GATHERSCATTER_HH
#define ALUGRID_SRC_SERIAL_GATHERSCATTER_HH

#include <set>

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
    virtual bool userDefinedPartitioning () const { return false ; }
    // return true if user defined load balancing weights are provided
    virtual bool userDefinedLoadWeights () const  { return false ; }
    // returns true if user defined partitioning needs to be readjusted 
    virtual bool repartition () { alugrid_assert (false); abort(); return false; }

    // this method is required for user defined partition 
    // returns true if the import ranks information was filled into set, false otherwise 
    // if false was returned a global communication is necessary to obtain this information
    virtual bool importRanks ( std::set< int >& ) const { return false ; }

    // this method is required for user defined partition 
    // returns true if the export ranks information was filled into set, false otherwise 
    // this information can be easily computed locally by the given destinations 
    virtual bool exportRanks ( std::set< int >& ) const { return false ; }

    // return load weight of given element 
    virtual int loadWeight( Gitter::helement_STI &elem ) { alugrid_assert (false); abort(); return 1; }

    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    virtual int destination( Gitter::helement_STI &elem ) { alugrid_assert (false); abort(); return -1; }

    virtual bool contains(int,int) const { alugrid_assert (false); abort(); return false; }

    // returns true if data handle contains user data for redistribution 
    virtual bool hasUserData () const { alugrid_assert (false); abort(); return false ; }

    virtual bool containsItem(const Gitter::helement_STI &elem ) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsItem(const Gitter::hface_STI   & elem ) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsItem(const Gitter::hedge_STI   & elem ) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsItem(const Gitter::vertex_STI & elem ) const { alugrid_assert (false); abort(); return false; }
    
    virtual bool containsInterior (const Gitter::hface_STI  & face , ElementPllXIF_t & elif) const { alugrid_assert (false); abort(); return false; }
    virtual bool containsGhost    (const Gitter::hface_STI  & face , ElementPllXIF_t & elif) const { alugrid_assert (false); abort(); return false; }
    
    virtual void inlineData ( ObjectStreamType & str , Gitter::helement_STI & elem, const int estimateElements ) { alugrid_assert (false); abort(); }
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

    virtual void compress () {}

    // dummy method to use GatherScatter as empty LoadBalanceHandle 
    template <class Entity>
    int operator () ( const Entity& ) const { alugrid_assert (false); abort(); return -1; }
  };

  typedef GatherScatter GatherScatterType;

} // namespace ALUGrid

#endif // #ifndef ALUGRID_SRC_SERIAL_GATHERSCATTER_HH

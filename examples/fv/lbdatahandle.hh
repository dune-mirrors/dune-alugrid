#ifndef LBDATAHANDLE_HH
#define LBDATAHANDLE_HH

#include <dune/alugrid/3d/datahandle.hh>
#include <dune/grid/common/datahandleif.hh>

template< class Grid, class DataHandleImpl, class Data >
class LoadBalanceDH
{
  typedef typename Grid :: Traits :: HierarchicIterator HierarchicIterator;

public:
  typedef typename Grid :: ObjectStreamType ObjectStream;

  typedef Dune::CommDataHandleIF< DataHandleImpl, Data > DataHandle;

  static const int dimension = Grid :: dimension;

  template< int codim >
  struct Codim
  {
    typedef typename Grid :: Traits :: template Codim< codim > :: Entity Entity;
    typedef typename Grid :: Traits :: template Codim< codim > :: EntityPointer
      EntityPointer;
  };

  typedef typename Codim< 0 > :: Entity Element;

private:
  const Grid &grid_;
  DataHandle &dataHandle_;

public:
  LoadBalanceDH ( const Grid &grid, DataHandle &dataHandle )
  : grid_( grid ),
    dataHandle_( dataHandle )
  {
  }

  void inlineData ( ObjectStream &stream, const Element &element ) const
  {
    inlineElementData( stream, element );

    const int maxLevel = grid_.maxLevel();
    const HierarchicIterator end = element.hend( maxLevel );
    for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
      inlineElementData( stream, *it );
  }

  void xtractData ( ObjectStream &stream, const Element &element, size_t newElements )
  {
    xtractElementData( stream, element );

    const int maxLevel = grid_.maxLevel();
    const HierarchicIterator end = element.hend( maxLevel );
    for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
      xtractElementData( stream, *it );
  }

  // return true if user defined partitioning methods should be used 
  bool userDefinedPartitioning () const 
  { 
    return true ; 
  }
  // return true if user defined load balancing weights are provided
  bool userDefinedLoadWeights () const 
  { 
    return false ; 
  }
  // returns true if user defined partitioning needs to be readjusted 
  bool repartition () const 
  { 
    angle_ += 2.*M_PI/100.;
    return true; 
  }
  // return load weight of given element 
  int loadWeight( const Element &element ) const 
  { 
    return 1;
  }
  // return destination (i.e. rank) where the given element should be moved to 
  // this needs the methods userDefinedPartitioning to return true
  int destination( const Element &element ) const 
  { 
    typename Element::Geometry::GlobalCoordinate w = element.geometry().center();
    double phi=arg(std::complex<double>(w[0],w[1]));
    if (w[1]<0) phi+=2.*M_PI;
    phi += angle_;
    int p = int(phi) % 6;
    // int p = int(w[0]*2.);
    return p;
  }

  void compress ()
  {}

private:
  void inlineElementData ( ObjectStream &stream, const Element &element ) const
  {
    // call element data direct without creating entity pointer
    if( dataHandle_.contains( dimension, 0 ) )
    {
      inlineEntityData<0>( stream, element ); 
    }

    // now call all higher codims 
    inlineCodimData< 1 >( stream, element );
    inlineCodimData< 2 >( stream, element );
    inlineCodimData< 3 >( stream, element );
  }

  void xtractElementData ( ObjectStream &stream, const Element &element )
  {
    // call element data direct without creating entity pointer
    if( dataHandle_.contains( dimension, 0 ) )
    {
      xtractEntityData<0>( stream, element ); 
    }

    // now call all higher codims 
    xtractCodimData< 1 >( stream, element );
    xtractCodimData< 2 >( stream, element );
    xtractCodimData< 3 >( stream, element );
  }

  template< int codim >
  void inlineCodimData ( ObjectStream &stream, const Element &element ) const
  {
    typedef typename Codim< codim > :: EntityPointer EntityPointer;

    if( dataHandle_.contains( dimension, codim ) )
    {
      const int numSubEntities = element.template count< codim >();
      for( int i = 0; i < numSubEntities; ++i )
      {
        const  EntityPointer pEntity = element.template subEntity< codim >( i );
        inlineEntityData< codim >( stream, *pEntity );
      }
    }
  }

  template< int codim >
  void xtractCodimData ( ObjectStream &stream, const Element &element )
  {
    typedef typename Codim< codim > :: EntityPointer EntityPointer;

    if( dataHandle_.contains( dimension, codim ) )
    {
      const int numSubEntities = element.template count< codim >();
      for( int i = 0; i < numSubEntities; ++i )
      {
        const  EntityPointer pEntity = element.template subEntity< codim >( i );
        xtractEntityData< codim >( stream, *pEntity );
      }
    }
  }

  template< int codim >
  void inlineEntityData ( ObjectStream &stream,
                          const typename Codim< codim > :: Entity &entity ) const
  {
    const size_t size = dataHandle_.size( entity );
    stream.write( size );
    dataHandle_.gather( stream, entity );
  }

  template< int codim >
  void xtractEntityData ( ObjectStream &stream,
                          const typename Codim< codim > :: Entity &entity )
  {
    size_t size = 0;
    stream.read( size );
    dataHandle_.scatter( stream, entity, size );
  }

  static double angle_;

};

template< class Grid, class DataHandleImpl, class Data >
double LoadBalanceDH<Grid,DataHandleImpl,Data>::angle_ = 0;

#endif

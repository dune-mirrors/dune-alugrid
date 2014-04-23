#ifndef GITTER_PLL_MGB_H_INCLUDED
#define GITTER_PLL_MGB_H_INCLUDED

#include <vector>

#include "../serial/serialize.h"
#include "../serial/gitter_mgb.h"

#include "gitter_pll_sti.h"
#include "gitter_pll_ldb.h"

namespace ALUGrid
{

  class VertexLinkage
  {
    typedef Gitter :: vertex_STI vertex_STI ;
    const LoadBalancer::DataBase& _db;
    std::vector< int > _linkage;
    const int _me ;
    const bool _computeVertexLinkage;
  public:
    VertexLinkage( const int me, 
                   const LoadBalancer::DataBase& db, 
                   const bool computeVertexLinkage )
      : _db( db ),
        _linkage(),
        _me( me ),
        _computeVertexLinkage( computeVertexLinkage )
    {}

    void compute( vertex_STI& vertex ) 
    {
      // clear existing vertex linkage 
      vertex.clearLinkage();

      if( vertex.isBorder() && _computeVertexLinkage )
      {
        typedef vertex_STI :: ElementLinkage_t ElementLinkage_t ;
        const ElementLinkage_t& linkedElements = vertex.linkedElements();
        const int elSize = linkedElements.size() ;
        std::set< int > linkage; 
        _linkage.resize( 0 );
        _linkage.reserve( elSize ); 
        for( int i=0; i<elSize; ++ i )
        {
          const int dest = _db.destination( linkedElements[ i ] ) ;
          assert( dest >= 0 );
          if( dest != _me )
          {
            linkage.insert( dest );
          }
        }

        typedef typename std::set< int >::iterator iterator ;
        const iterator end = linkage.end();
        // create sorted vector containing each entry only once
        for( iterator it = linkage.begin(); it != end; ++ it )
          _linkage.push_back( *it );

        // set linkage 
        vertex.setLinkageSorted( _linkage );
      }
    }
  };



  class ParallelGridMover
  : public MacroGridBuilder
  {
    protected :
      void unpackVertex (ObjectStream &);
      void unpackHedge1 (ObjectStream &);
      void unpackHface3 (ObjectStream &);
      void unpackHface4 (ObjectStream &);
      void unpackHexa (ObjectStream &, GatherScatterType* );
      void unpackTetra (ObjectStream &, GatherScatterType* );
      void unpackPeriodic3 (ObjectStream &);
      void unpackPeriodic4 (ObjectStream &);
      void unpackHbnd3Int (ObjectStream &);
      void unpackHbnd3Ext (ObjectStream &);
      void unpackHbnd4Int (ObjectStream &);
      void unpackHbnd4Ext (ObjectStream &);

      // creates Hbnd3IntStorage with ghost info if needed 
      bool InsertUniqueHbnd3_withPoint (int (&)[3],
                                        Gitter::hbndseg::bnd_t,
                                        int ldbVertexIndex,
                                        int master,
                                        MacroGhostInfoTetra* );
    
      // creates Hbnd4IntStorage with ghost info if needed 
      bool InsertUniqueHbnd4_withPoint (int (&)[4], 
                                        Gitter::hbndseg::bnd_t, 
                                        int ldbVertexIndex,
                                        int master,
                                        MacroGhostInfoHexa* );


      // former constructor 
      void initialize ();
      void finalize (); 

    public :
      ParallelGridMover (BuilderIF &, VertexLinkage& vxLinkage );
      // unpack all elements from the stream 
      void unpackAll (ObjectStream &, GatherScatterType*);
      void packAll   (const int link, ObjectStream &, GatherScatterType* );
      // unpack all elements from all streams
      // void unpackAll (std::vector< ObjectStream > &, GatherScatterType* );

      ~ParallelGridMover ();
    protected:
      VertexLinkage& _vxLinkage ;
      using MacroGridBuilder :: reserve ;
      using MacroGridBuilder :: clear ;
  };

} // namespace ALUGrid

#endif // #ifndef GITTER_PLL_MGB_H_INCLUDED

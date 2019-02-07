#ifndef GITTER_PLL_MGB_H_INCLUDED
#define GITTER_PLL_MGB_H_INCLUDED

#include <vector>

#include "../serial/serialize.h"
#include "../serial/gitter_mgb.h"

#include "gitter_pll_sti.h"
#include "gitter_pll_ldb.h"

namespace ALUGrid
{


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

      ProjectVertexPtr unpackVertexProjection(ObjectStream &);

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
      ParallelGridMover (BuilderIF &  );
      // unpack all elements from the stream
      void unpackAll (ObjectStream &, GatherScatterType*, const int );
      void packAll   (const int link, ObjectStream &, GatherScatterType* );
      // unpack all elements from all streams
      // void unpackAll (std::vector< ObjectStream > &, GatherScatterType* );

      ~ParallelGridMover ();
    protected:
      using MacroGridBuilder :: reserve ;
      using MacroGridBuilder :: clear ;
  };

} // namespace ALUGrid

#endif // #ifndef GITTER_PLL_MGB_H_INCLUDED

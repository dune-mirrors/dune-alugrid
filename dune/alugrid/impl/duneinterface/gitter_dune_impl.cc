// Robert Kloefkorn (c) 2004 - 2005
#include <config.h>

#include <fstream>

#include "gitter_dune_impl.h"

namespace ALUGrid
{

  IteratorSTI < Gitter::helement_STI > * GitterDuneImpl::
  leafIterator (const helement_STI *)
  {
    return new Insert < PureElementAccessIterator < Gitter::helement_STI >::Handle,
    TreeIterator < Gitter::helement_STI, is_leaf < Gitter::helement_STI> > > (container ());
  }

  IteratorSTI < Gitter::helement_STI > * GitterDuneImpl::
  leafIterator (const IteratorSTI < helement_STI > * p)
  {
    return new Insert < PureElementAccessIterator < Gitter::helement_STI >::Handle,
    TreeIterator < Gitter::helement_STI, is_leaf < Gitter::helement_STI> > >
    (*(const Insert < PureElementAccessIterator < Gitter::helement_STI >::Handle,
    TreeIterator < Gitter::helement_STI, is_leaf < Gitter::helement_STI> > > *) p);
  }

  int GitterDuneBasis::preCoarsening  (Gitter::helement_STI & elem)
  {
    // if _arp is set then the extrenal preCoarsening is called
    return (_arp) ? (*_arp).preCoarsening(elem) : 0;
  }

  int GitterDuneBasis::postRefinement (Gitter::helement_STI & elem)
  {
    // if _arp is set then the extrenal postRefinement is called
    return (_arp) ? (*_arp).postRefinement(elem) : 0;
  }

  int GitterDuneBasis::preCoarsening  (Gitter::hbndseg_STI & bnd)
  {
    // if _arp is set then the extrenal preCoarsening is called
    return (_arp) ? (*_arp).preCoarsening( bnd ) : 0;
  }

  int GitterDuneBasis::postRefinement (Gitter::hbndseg_STI & bnd)
  {
    // if _arp is set then the extrenal postRefinement is called
    return (_arp) ? (*_arp).postRefinement( bnd ) : 0;
  }

  void GitterDuneBasis ::
  setAdaptRestrictProlongOp( AdaptRestrictProlongType & arp )
  {
    if( _arp )
      std::cerr << "WARNING (ignored): _arp not null in GitterDuneBasis::setAdaptRestrictProlongOp." << std::endl;
    _arp = &arp;
  }

  void GitterDuneBasis::removeAdaptRestrictProlongOp()
  {
    _arp = 0;
  }

  bool GitterDuneBasis::duneAdapt (AdaptRestrictProlongType & arp)
  {
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterDuneBasis::duneAdapt ()" << std::endl, 1) : 1);
    // set restriction-prolongation callback obj
    setAdaptRestrictProlongOp(arp);
    // call adapt method
    const bool refined = this->adapt();
    // sets pointer to zero
    removeAdaptRestrictProlongOp ();
    return refined;
  }

} // namespace ALUGrid

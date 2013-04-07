#ifndef DUNE_ALUGRID_3D_DATACOLLECTORCAPS_HH
#define DUNE_ALUGRID_3D_DATACOLLECTORCAPS_HH

namespace ALUGrid
{

  namespace DataCollectorCaps
  {

    template< class DataCollector >
    class HasUserDefinedPartitioning
    {
      typedef char Small;
      struct Big { char dummy[2]; };

      template< class T, T > struct TypeCheck;

      typedef bool (DataCollector::*Method)() const;

      template< class T >
      static Small test ( TypeCheck< Method, &T::userDefinedPartitioning > * );
      template< class T >
      static Big test ( ... );

    public:
      static const bool v = (sizeof( test< DataCollector >( 0 ) ) == sizeof( Small ));
    };

    template< class DataCollector >
    class HasUserDefinedLoadWeights
    {
      typedef char Small;
      struct Big { char dummy[2]; };

      template< class T, T > struct TypeCheck;

      typedef bool (DataCollector::*Method)() const;

      template< class T >
      static Small test ( TypeCheck< Method, &T::userDefinedLoadWeights > * );
      template< class T >
      static Big test ( ... );

    public:
      static const bool v = (sizeof( test< DataCollector >( 0 ) ) == sizeof( Small ));
    };

  } // namespace DataCollectorCaps

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_3D_DATACOLLECTORCAPS_HH

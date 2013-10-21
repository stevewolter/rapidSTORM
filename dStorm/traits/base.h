#ifndef DSTORM_TRAITS_BASE_H
#define DSTORM_TRAITS_BASE_H

#include "../namespaces.h"
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/dimensionless.hpp>
#include <Eigen/Core>
#include "../pair_Eigen_traits.h"
#include <boost/units/Eigen/Core>
#include <boost/optional/optional.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/numeric/interval/interval.hpp>

namespace dStorm {
namespace traits {

template <int Rows, int Cols>
struct MatrixOrScalar;

template <>
struct MatrixOrScalar<1,1> {
    template <typename Scalar>
    struct value { 
        typedef Scalar type;
        static void init( type& t, const Scalar& v ) { t = v; }
    };

    static const bool is_scalar = true;

  public:
    template <class Functor>
    static typename Functor::result_type  
        apply( Functor& f, typename Functor::argument_type t ) { return f(t); }
    template <class Functor>
    static typename Functor::result_type  
        apply( 
            Functor& f,
            typename Functor::first_argument_type t1,
            typename Functor::second_argument_type t2 ) { return f(t1, t2); }
};

template <typename Type, int _Rows = 1, int _Cols = 1>
struct Value {
  public:
    typedef MatrixOrScalar<_Rows,_Cols> MoS;
    typedef typename MoS::template value<Type>::type ValueType;

  private:
    typedef typename MoS::template value<bool> MoSb;

  public:
    static const bool has_value = true, is_scalar = MoS::is_scalar;
    static const int Rows = _Rows, Cols = _Cols;
    typedef Type OutputType;
    typedef Type Scalar;
    typedef typename MoSb::type IsGivenType;
    IsGivenType is_given;

    boost::optional<ValueType> static_value;
    Value() { MoSb::init(is_given, false); }
};

template <typename Type>
struct value;

template <typename Type>
struct derived_value;

template <typename Scalar, int Rows, int Cols, int Flags>
struct derived_value< Eigen::Matrix<Scalar,Rows,Cols,Flags> >
: public Value< Scalar, Rows, Cols > {};

}
}

#endif

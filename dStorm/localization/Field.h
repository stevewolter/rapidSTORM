#ifndef DSTORM_LOCALIZATION_FIELD_H
#define DSTORM_LOCALIZATION_FIELD_H

#include <cassert>
#include "../namespaces.h"
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/utility/enable_if.hpp>

#include "Traits.h"

namespace dStorm {
namespace localization {

namespace impl {

template <typename Type>
struct SIizer {
    template <typename Resolution>
    static float get( const Type& value, const Resolution& resolution ) { 
        assert( resolution.is_set() );
        return double(value / *resolution / si::metre); 
    }
};

template <typename NumType>
struct SIizer< quantity< si::length, NumType > > {
    template <typename Resolution>
    static float get( const quantity< si::length, NumType >& value, const Resolution& )
        { return value / si::metre; }
};

template <typename Type>
struct Accessor {
    typedef Type type;
    static type& get(Type& t) { return t; }
    static const type& get(const Type& t) { return t; }
};

template <typename Scalar, int Flags, int MR, int MC>
struct Accessor< Eigen::Matrix<Scalar, 1, 1, Flags, MR, MC> >
: public Accessor<Scalar>
{
    typedef Scalar type;
    static type& get(Eigen::Matrix<Scalar, 1, 1, Flags, MR, MC>& t) { return t.data()[0]; }
    static const type& get(const Eigen::Matrix<Scalar, 1, 1, Flags, MR, MC>& t) { return t.data()[0]; }
};

}

template <typename TraitsType>
struct Field
{
    typedef TraitsType Traits;
    typedef typename Traits::ValueType Type;
  private:
    Type _value, _uncertainty;

  public:
    Field() : _value(Traits::default_value) {}
    Field( const Type& value ) : _value(value) {}
    Field( const Type& value, const Type& uncertainty ) 
        : _value(value), _uncertainty(uncertainty) {}

    Field<Traits>& operator=(const Type& t) { _value = t; return *this; }
    const Type& operator()() const { return _value; }
    Type& operator()() { return _value; }

    const Type& value() const { return _value; }
    Type& value() { return _value; }
    const Type& uncertainty() const { return _uncertainty; }
    Type& uncertainty() { return _uncertainty; }

    float in_nm( const Traits& t ) const
        { return impl::SIizer<Type>::get(_value, t.resolution); }
    float uncertainty_in_nm( const Traits& t ) const
        { return impl::SIizer<Type>::get(_uncertainty, t.resolution); }

    const Traits& traits( const Traits& t ) const { return t; }
};

}
}

#endif

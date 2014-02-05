#ifndef DSTORM_LOCALIZATION_FIELD_H
#define DSTORM_LOCALIZATION_FIELD_H

#include <cassert>
#include "dStorm/namespaces.h"
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/utility/enable_if.hpp>

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

}

template <typename TraitsType>
struct Field
{
    typedef TraitsType Traits;
    typedef typename Traits::ValueType Type;
  private:
    Type _value;

  public:
    Field() : _value(Traits::default_value) {}
    Field( const Type& value ) : _value(value) {}

    Field<Traits>& operator=(const Type& t) { _value = t; return *this; }
    const Type& operator()() const { return _value; }
    Type& operator()() { return _value; }

    const Type& value() const { return _value; }
    Type& value() { return _value; }

    float in_nm( const Traits& t ) const
        { return impl::SIizer<Type>::get(_value, t.resolution); }

    const Traits& traits( const Traits& t ) const { return t; }
};

}
}

#endif

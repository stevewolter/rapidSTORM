#ifndef DSTORM_TRAITS_SCALAR_H
#define DSTORM_TRAITS_SCALAR_H

#include <list>
#include "base.h"
#include "../Localization_decl.h"
#include "../localization/Field.h"
#include <boost/fusion/include/at_c.hpp>

namespace dStorm {
namespace traits {

std::string axis_name(int index);

template <typename TraitsType, bool scalar_type = TraitsType::is_scalar>
struct Scalar;

template <typename TraitsType>
struct Scalar<TraitsType, true>
{
    Scalar() {}
    Scalar(int, int) {}
    static std::list<Scalar> all_scalars() { 
        std::list<Scalar> rv;
        rv.push_back(Scalar());
        return rv;
    }

    typedef TraitsType Traits;
    typedef typename TraitsType::ValueType value_type;
    typedef typename TraitsType::RangeType range_type;

    const value_type& value(const value_type& r) const { return r; }
    value_type& value(value_type& r) const { return r; }

    bool is_given( const TraitsType& t ) const { return t.is_given; }
    bool& is_given( TraitsType& t ) const { return t.is_given; }

    range_type range(const range_type& r) const { return r; }
    range_type& range(range_type& r) const { return r; }
    range_type range(const TraitsType& r) const { return r.range(); }
    range_type& range(TraitsType& r) const { return r.range(); }

    const int row() const { return 0; }
    const int column() const { return 0; }

    struct Iterator;
    static Iterator begin();
    static Iterator end();

    template <typename Tag>
    struct result_of { 
        typedef typename Tag::template in<TraitsType>::type get; 
        typedef typename Tag::template in<TraitsType>::type set; 
    };

    template <typename Tag>
    typename Tag::template in<TraitsType>::type get( const TraitsType& t ) const
        { return Tag::template in<TraitsType>::get(t); }
    template <typename Tag>
    typename Tag::template in<TraitsType>::type& set( TraitsType& t ) const
        { return Tag::template in<TraitsType>::set(t); }
    template <typename Tag>
    typename Tag::template in<TraitsType>::type get( const localization::Field<TraitsType>& t ) const
        { return Tag::template in<TraitsType>::get(t); }
    template <typename Tag>
    typename Tag::template in<TraitsType>::type& set( localization::Field<TraitsType>& t ) const
        { return Tag::template in<TraitsType>::set(t); }

    template <typename Tag, int Field>
    typename Tag::template in<TraitsType>::type get_field( const Localization& t ) const
        { return Tag::template in<TraitsType>::get(boost::fusion::at_c<Field>(t)); }
    template <typename Tag, int Field>
    typename Tag::template in<TraitsType>::type& set_field( Localization& t ) const
        { return Tag::template in<TraitsType>::set(boost::fusion::at_c<Field>(t)); }

    template <typename Tag>
    std::string shorthand() const
        { return Tag::template in<TraitsType>::shorthand(); }
};

template <typename TraitsType>
class Scalar< TraitsType, false > 
{
    int r, c;
 
  public:
    const int row() const { return r; }
    const int column() const { return c; }

    Scalar(int r, int c) : r(r), c(c) {}
    static std::list<Scalar> all_scalars() { 
        std::list<Scalar> rv;
        for (int r = 0; r < TraitsType::ValueType::RowsAtCompileTime; ++r)
            for (int c = 0; c < TraitsType::ValueType::ColsAtCompileTime; ++c)
                rv.push_back(Scalar(r, c));
        return rv;
    }

    typedef TraitsType Traits;
    typedef typename TraitsType::ValueType::Scalar value_type;
    typedef typename TraitsType::RangeType::Scalar range_type;

    value_type value(const typename TraitsType::ValueType& t) const { return t(r,c); }
    value_type& value(typename TraitsType::ValueType& t) const { return t(r,c); }

    bool is_given( const TraitsType& t ) const { return t.is_given(r,c); }
    bool& is_given( TraitsType& t ) const { return t.is_given(r,c); }

    range_type range(const typename TraitsType::RangeType& t) const { return t(r,c); }
    range_type& range(typename TraitsType::RangeType& t) const { return t(r,c); }
    range_type range(const TraitsType& t) const { return t.range()(r,c); }
    range_type& range(TraitsType& t) const { return t.range()(r,c); }

    struct Iterator;
    static Iterator begin();
    static Iterator end();

    template <typename Tag>
    struct result_of { 
        typedef typename Tag::template in<TraitsType>::type::Scalar get; 
        typedef typename Tag::template in<TraitsType>::type::Scalar set; 
    };

    template <typename Tag>
    typename Tag::template in<TraitsType>::type::Scalar get( const TraitsType& t ) const
        { return Tag::template in<TraitsType>::get(t)(r,c); }
    template <typename Tag>
    typename Tag::template in<TraitsType>::type::Scalar& set( TraitsType& t ) const
        { return Tag::template in<TraitsType>::set(t)(r,c); }
    template <typename Tag>
    typename Tag::template in<TraitsType>::type::Scalar get( const Localization& t ) const
        { return Tag::template in<TraitsType>::get(t)(r,c); }
    template <typename Tag>
    typename Tag::template in<TraitsType>::type::Scalar& set( Localization& t ) const
        { return Tag::template in<TraitsType>::set(t)(r,c); }

    template <typename Tag, int Field>
    typename Tag::template in<TraitsType>::type::Scalar get_field( const Localization& t ) const
        { return Tag::template in<TraitsType>::get(boost::fusion::at_c<Field>(t))(r,c); }
    template <typename Tag, int Field>
    typename Tag::template in<TraitsType>::type::Scalar& set_field( Localization& t ) const
        { return Tag::template in<TraitsType>::set(boost::fusion::at_c<Field>(t))(r,c); }

    template <typename Tag>
    std::string shorthand() const
        { return Tag::template in<TraitsType>::shorthand() 
           + ((TraitsType::Rows > 1) ? axis_name(row()) : "") + ((TraitsType::Cols > 1) ? axis_name(column()) : ""); }
};

}
}

#endif

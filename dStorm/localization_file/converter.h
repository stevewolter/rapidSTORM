#ifndef DSTORM_LOCALIZATION_FILE_CONVERTER_H
#define DSTORM_LOCALIZATION_FILE_CONVERTER_H

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <boost/units/unit.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/io.hpp>

namespace boost {
namespace units {

template <typename Quantity>
struct ValueConverter {
    static inline boost::shared_ptr<ValueConverter> create(const std::string& unit_name);
    template <typename Factor>
    static inline boost::shared_ptr<ValueConverter> create(const std::string& unit_name, const Factor&);
    virtual ~ValueConverter() {}
    virtual Quantity from_value( const typename Quantity::value_type& v ) = 0;
};

template <typename Quantity, typename Operator, int Power>
struct OperatorConverter: public ValueConverter<Quantity> {
    Operator o;
    typedef typename Operator::memory_type Argument;
    typedef quantity< typename make_scaled_unit< typename Operator::argument_unit, scale<10, static_rational<Power> > >::type, typename Quantity::value_type > ScaledQuantity;

    typedef ValueConverter<Quantity> Base;
    OperatorConverter(const Argument& a) : o(a) {}
    ~OperatorConverter() {}

    static inline boost::shared_ptr<Base> create(const std::string& unit_name, const Argument& a) {
        typename ScaledQuantity::unit_type u;
        std::string in_parens = "(" + unit_name + ")";
        if ( unit_name == name_string(u) || unit_name == symbol_string(u) 
             || (name_string(u) == "" && unit_name == "dimensionless") 
             || name_string(u) == in_parens || symbol_string(u) == in_parens) {
            return boost::shared_ptr<Base>( new OperatorConverter(a) );
        } else {
            return OperatorConverter<Quantity,Operator,Power+3>::create(unit_name, a);
        }
    }

    Quantity from_value( const typename Quantity::value_type& v ) {
        typedef quantity< typename Operator::argument_unit, typename Quantity::value_type > UnscaledQuantity;
        UnscaledQuantity s = UnscaledQuantity::from_value(v * pow(10.0, Power));
        return o( s );
    }
};

template <typename Quantity, typename Operator>
struct OperatorConverter<Quantity, Operator, 18> {
    typedef ValueConverter<Quantity> Base;
    static inline boost::shared_ptr<Base> create(const std::string&, const typename Operator::memory_type&) {
            return boost::shared_ptr<Base>();
    }
};

template <typename Quantity>
struct NullOperator {
    typedef int memory_type;
    typedef typename Quantity::unit_type argument_unit;
    NullOperator(int) {}
    Quantity operator()( const Quantity& a ) { return a; }
};

template <typename Quantity, typename Factor>
struct MultiplicationOperator {
    typedef Factor memory_type;
    typedef typename divide_typeof_helper<typename Quantity::unit_type, typename Factor::unit_type>::type argument_unit;

    Factor f;
    MultiplicationOperator(const Factor& f) : f(f) {}
    Quantity operator()( const quantity<argument_unit, typename Quantity::value_type>& a ) 
        { return Quantity(a * f); }
};

template <typename Quantity, typename Factor>
struct DivisorOperator {
    typedef Factor memory_type;
    typedef typename multiply_typeof_helper<typename Quantity::unit_type, typename Factor::unit_type>::type argument_unit;

    Factor f;
    DivisorOperator(const Factor& f) : f(f) {}
    Quantity operator()( const quantity<argument_unit, typename Quantity::value_type>& a ) 
        { return Quantity(a / f); }
};

template <typename Quantity>
boost::shared_ptr<ValueConverter<Quantity> > ValueConverter<Quantity>::create(const std::string& unit_name)
{
    return OperatorConverter<Quantity, NullOperator<Quantity>, -15>::create( unit_name, 0 );
}

template <typename Quantity>
template <typename Scale>
boost::shared_ptr< ValueConverter<Quantity> >
ValueConverter<Quantity>::create(const std::string& unit_name, const Scale& scale)
{
    boost::shared_ptr< ValueConverter<Quantity> > rv = create(unit_name);
    if ( rv.get() != NULL ) return rv;
    rv = OperatorConverter<Quantity, MultiplicationOperator<Quantity,Scale>, -15>::create( unit_name, scale );
    if ( rv.get() != NULL ) return rv;
    rv = OperatorConverter<Quantity, DivisorOperator<Quantity,Scale>, -15>::create( unit_name, scale );
    if ( rv.get() != NULL ) return rv;
    
    return rv;
}

}
}

#endif

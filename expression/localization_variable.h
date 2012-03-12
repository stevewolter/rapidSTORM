#ifndef DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_H
#define DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_H

#include "localization_variable_decl.h"
#include "types.h"
#include <boost/fusion/include/value_at.hpp>
#include <dStorm/traits/scalar.h>
#include "QuantityDynamizer.hpp"

namespace dStorm {
namespace expression {

template <typename Type>
struct optional_removed {
    typedef Type type; 
};
template <typename Type>
struct optional_removed< boost::optional<Type> > { typedef Type type; };


template <int Field, typename Tag>
struct LocalizationVariable : public Variable
{
    typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Field> >::type::Traits TraitsType;
    typedef typename Tag::template in<TraitsType> TaggedTraits;
    typedef traits::Scalar<TraitsType> Scalar;
  private:
    const Scalar scalar;
    typedef typename optional_removed<typename Scalar::template result_of<Tag>::set>::type my_quantity;
    const QuantityDynamizer< my_quantity > dynamizer;

  public:
    LocalizationVariable( const Scalar& );
    LocalizationVariable* clone() const { return new LocalizationVariable(*this); }
    bool is_static( const input::Traits<Localization>& traits) const;
    DynamicQuantity get( const input::Traits<Localization>& traits ) const; 
    DynamicQuantity get( const Localization& l ) const; 
    void set( input::Traits<Localization>&, const DynamicQuantity& ) const;
    bool set( const input::Traits<Localization>&, Localization&, const DynamicQuantity& ) const;
    static std::string name(const Scalar& s);
};

}
}

#endif

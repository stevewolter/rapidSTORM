#ifndef DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_H
#define DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_H

#include "localization_variable_decl.h"
#include "types.h"
#include <boost/fusion/include/value_at.hpp>
#include <dStorm/traits/scalar.h>

namespace dStorm {
namespace expression {

template <int Field, typename Tag>
struct Variable : public variable
{
    typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Field> >::type::Traits TraitsType;
    typedef typename Tag::template in<TraitsType> TaggedTraits;
    typedef traits::Scalar<TraitsType> Scalar;
  private:
    Scalar scalar;
    DynamicUnit unit;
    double scale;

  public:
    Variable( const Scalar& );
    Variable* clone() const { return new Variable(*this); }
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

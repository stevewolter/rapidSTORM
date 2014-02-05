#ifndef BOOST_UNITS_NUMERIC_INTERVAL_HPP
#define BOOST_UNITS_NUMERIC_INTERVAL_HPP

#include <boost/numeric/interval/checking.hpp>
#include <boost/units/quantity.hpp>

namespace boost {
namespace numeric {
namespace interval_lib {

template <class U, class V>
class checking_base< boost::units::quantity<U,V> >
: public checking_base<V>
{
  typedef boost::units::quantity<U,V> Qty;
  typedef checking_base<V> Base;

 public:
  static Qty pos_inf()
    { return Qty::from_value( Base::pos_inf() ); }

  static Qty neg_inf()
    { return Qty::from_value( Base::neg_inf() ); }

  static Qty nan()
    { return Qty::from_value( Base::nan() ); }

  static Qty empty_lower()
    { return Qty::from_value( Base::empty_lower() ); }

  static Qty empty_upper()
    { return Qty::from_value( Base::empty_upper() ); }

};

}
}
}

#endif

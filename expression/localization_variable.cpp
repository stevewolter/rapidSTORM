#include "localization_variable_impl.h"
#include <boost/mpl/for_each.hpp>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/traits/range_impl.h>

namespace dStorm {
namespace expression {

template <int Field>
struct FieldAdder : public FieldAdder<Field+1> {
    boost::ptr_vector<variable>& target;
    FieldAdder( boost::ptr_vector<variable>& target )
        : FieldAdder<Field+1>(target), target(target) {}

    template <typename Tag>
    void operator()(const Tag&) const {
        typedef Variable<Field,Tag> Target;
        typedef typename Target::Scalar Scalar;
        for ( typename Scalar::Iterator i = Scalar::begin(); i != Scalar::end(); ++i )
            target.push_back( new Target(*i) );
    }

    void add_variables_for_field() const {
        boost::mpl::for_each<traits::tags>( *this );
        FieldAdder<Field+1>::add_variables_for_field();
    }
};

template <>
struct FieldAdder<Localization::Fields::Count> {
    FieldAdder( boost::ptr_vector<variable>& ) {}
    void add_variables_for_field() const {}
};

std::auto_ptr< boost::ptr_vector<variable> >
variables_for_localization_fields() {
    boost::ptr_vector<variable> rv;
    FieldAdder<0>(rv).add_variables_for_field();
    return rv.release();
}

}
}

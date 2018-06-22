#include "calibrate_3d/ZTruth.h"
#include "expression/Filter.h"
#include "expression/VariableLValue.h"
#include "expression/Variable.h"
#include "expression/QuantityDynamizer.hpp"

namespace dStorm {
namespace calibrate_3d {

/** \todo TrueZ saves its state in the two kernel improvement field.
 *        This is a pretty dirty hack, even though it doesn't hurt for now. */
class TrueZ 
: public expression::Variable {
    expression::QuantityDynamizer< samplepos::Scalar > reader;
public:
    TrueZ() : Variable("truez") {}
    const samplepos::Scalar value(const Localization& l) const { 
        return samplepos::Scalar::from_value( l.two_kernel_improvement() );
    }

    Variable* clone() const { assert( false ); return new TrueZ(); }
    bool is_static( const input::Traits<Localization>& ) const { return false; }
    expression::DynamicQuantity get( const input::Traits<Localization>& ) const 
        { return reader.from_value( std::numeric_limits<double>::quiet_NaN() ); }
    expression::DynamicQuantity get( const Localization& l ) const 
        { return reader( value(l) ); }
    void set( input::Traits<Localization>&, const expression::DynamicQuantity& ) const {}
    bool set( const input::Traits<Localization>&, Localization& l, const expression::DynamicQuantity& v ) const { 
        l.two_kernel_improvement() = reader(v).value();
        return true;
    }
    
};

ZTruth::ZTruth( const std::string& filter_expression, const std::string& true_z_expression )
{
    std::auto_ptr<TrueZ> my_new_z( new TrueZ() );
    new_z_variable = my_new_z.get();
    parser.add_variable( std::auto_ptr<expression::Variable>(my_new_z) );
    if ( filter_expression != "" )
        filter = expression::source::make_filter( filter_expression, parser );
    if ( true_z_expression == "" )
        throw std::runtime_error("An expression for the true Z value must be given to the 3D calibrator");
    new_z_expression = expression::source::make_variable_lvalue( *new_z_variable, true_z_expression, parser );
}

void ZTruth::set_meta_info( const input::Traits<Localization>& a )
{
    localization_traits = a;
    if ( filter.get() )
        filter->announce( parser.get_variable_table(), localization_traits );
    new_z_expression->announce( parser.get_variable_table(), localization_traits );
}

output::LocalizedImage::iterator ZTruth::calibrate( output::LocalizedImage& c )
{
    output::LocalizedImage::iterator end = c.end();
    end = new_z_expression->evaluate(
        parser.get_variable_table(), localization_traits,
        c.begin(), end );
    if ( filter.get() ) 
        end = filter->evaluate( parser.get_variable_table(), localization_traits,
                                c.begin(), end );
    return end;
}

quantity<si::length> ZTruth::true_z( const Localization& l ) const {
    return new_z_variable->value(l);
}

}
}

#include "debug.h"
#include <boost/units/io.hpp>
#include <sstream>
#include <dStorm/output/Traits.h>

#include "localization_field_impl.h"
#include "unknown_field.h"
#include "children_field.h"

namespace dStorm {
namespace localization_file {

using namespace boost::fusion;


const std::string type_string<float>::ident ()
    { return "floating point with . for decimals and "
      "optional scientific e-notation"; }

const std::string type_string<double>::ident()
    { return type_string<float>::ident(); }

const std::string type_string<int>::ident()
    { return "integer"; }

const std::string str(int i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}

class CovarianceMatrixField
: public Field 
{
    Direction dir;

    void write( std::ostream& output, const Localization& source )
        { throw std::logic_error("Read-only field written"); }
    void parse(std::istream& input, Localization& target) {
        float value;
        input >> value;
        quantity<si::length, float> fwhm( sqrt(value) * 1E-6 * 2.35 * si::meter );
        if (dir == 0) {
          target.psf_width_x() = fwhm;
        } else {
          target.psf_width_y() = fwhm;
        }
    }
    std::auto_ptr<TiXmlNode> makeNode( const Traits& traits ) 
        { throw std::logic_error("Read-only field written"); }
    Field* clone() const { return new CovarianceMatrixField(*this); }
public:
    CovarianceMatrixField( Direction dir ) : dir(dir) {}
    static Ptr parse(const TiXmlElement& node, Traits& traits ) {
        const char* ident_attrib = node.Attribute("identifier"), *unit_attrib = node.Attribute("unit");
        if ( ! ident_attrib || ! unit_attrib || std::string( unit_attrib ) != "pico(meter^2)" )
            return Ptr();

        Direction dir;
        if ( std::string( ident_attrib ) == "PSFCovarMatrix-0-0" ) {
            dir = Direction_X;
            traits.psf_width_x().is_given = true;
        } else if ( std::string( ident_attrib ) == "PSFCovarMatrix-1-1" ) {
            dir = Direction_Y;
            traits.psf_width_y().is_given = true;
        } else {
            return Ptr();
        }

        return Ptr( new CovarianceMatrixField(dir) );
    }

};

template <int Index>
Field* Field::create_interface( const TiXmlElement& node, input::Traits<Localization>& traits )
{
    typedef typename result_of::value_at<Localization, boost::mpl::int_<Index> >::type::Traits Traits;
    Field::Ptr p = LocalizationField< Index >::try_to_parse(node, traits);
    if ( p.get() )
        return p.release();
    else
        return create_interface<Index+1>( node, traits );
}

template <>
Field* Field::create_interface<Localization::Fields::Count>( const TiXmlElement& node, input::Traits<Localization>& t )
{
    std::auto_ptr<Field> f = CovarianceMatrixField::parse(node,t);
    if ( f.get() ) return f.release();

    const char* syntax_attrib = node.Attribute("syntax");
    if ( syntax_attrib == NULL )
        throw std::runtime_error("Field is missing "
            "syntax attribute.");
    std::string syntax = syntax_attrib;

    if ( syntax == type_string<int>::ident() )
        return new Unknown<int>();
    else if ( syntax == type_string<double>::ident() )
        return new Unknown<double>();
    else if ( syntax == type_string<float>::ident() )
        return new Unknown<float>();
    else
        throw std::runtime_error("Unknown syntax " + syntax + " in localization file field.");
}

Field::Ptr 
Field::parse(const TiXmlNode& node, input::Traits<Localization>& traits)
{
    const TiXmlElement* element = dynamic_cast<const TiXmlElement*>(&node);
    if ( ! element )
        return Field::Ptr();
    else if ( element->Value() == std::string("field") )
        return Ptr( create_interface<0>( *element, traits ) );
    else if ( element->Value() == std::string("localizations") )
        return Ptr( new ChildrenField( *element, traits ) );
    else
        return Ptr(NULL);
}

template <int Field>
static void create_localization_fields_recursive( const input::Traits<Localization>& traits, Field::Fields& result );

template <>
void create_localization_fields_recursive<Localization::Fields::Count>(const Field::Traits&, Field::Fields&) {
}

template <int LField>
void create_localization_fields_recursive( const input::Traits<Localization>& traits, Field::Fields& result )
{
    result.push_back(new LocalizationField<LField>());
    create_localization_fields_recursive<LField+1>(traits, result);
}

void create_localization_fields( const Field::Traits& traits, Field::Fields& result )
{
    create_localization_fields_recursive<0>( traits, result );
}

std::auto_ptr<Field> Field::construct_xyztI( const Field::Traits& traits )
{
    std::auto_ptr<ChildrenField> rv( new ChildrenField(traits) );
    rv->add_field( new LocalizationField< Localization::Fields::PositionX >() );
    rv->add_field( new LocalizationField< Localization::Fields::PositionY >() );
    if ( traits.position_z().is_given )
        rv->add_field( new LocalizationField< Localization::Fields::PositionZ >() );
    rv->add_field( new LocalizationField< Localization::Fields::ImageNumber >() );
    rv->add_field( new LocalizationField< Localization::Fields::Amplitude >() );
    return std::auto_ptr<Field>(rv.release());
}

std::auto_ptr<Field> Field::construct( const Field::Traits& traits )
{
    return std::auto_ptr<Field>( new ChildrenField(traits, 0) );
}

}
}

namespace boost {
namespace units {

template <>
std::string name_string( const quantity<si::dimensionless, float>& ) {
    return "dimensionless";
}

}
}

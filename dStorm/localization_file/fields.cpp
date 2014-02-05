#include "debug.h"
#include <boost/units/io.hpp>
#include <boost/bind/bind.hpp>
#include <boost/mpl/for_each.hpp>
#include <sstream>
#include <dStorm/output/Traits.h>

#include "dStorm/localization/Fields.h"
#include "dStorm/localization_file/localization_field_impl.h"
#include "dStorm/localization_file/unknown_field.h"
#include "dStorm/localization_file/children_field.h"

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

class FieldCreator {
  public:
    FieldCreator(const TiXmlElement& node, input::Traits<Localization>& traits)
        : node(node), traits(traits) {}

    Field::Ptr get_result();

    template <typename Tag>
    void operator()(Tag tag) {
        if (!named_field.get()) {
            named_field = LocalizationField< Tag >::try_to_parse(node, traits.field(tag));
        }
    }

  private:
    const TiXmlElement& node;
    input::Traits<Localization>& traits;
    Field::Ptr named_field;
};

Field::Ptr FieldCreator::get_result()
{
    boost::mpl::for_each<localization::Fields>(boost::ref(*this));
    if (named_field.get()) {
        return named_field;
    }

    std::auto_ptr<Field> f = CovarianceMatrixField::parse(node, traits);
    if ( f.get() ) return f;

    const char* syntax_attrib = node.Attribute("syntax");
    if ( syntax_attrib == NULL )
        throw std::runtime_error("Field is missing "
            "syntax attribute.");
    std::string syntax = syntax_attrib;

    if ( syntax == type_string<int>::ident() )
        return Field::Ptr(new Unknown<int>());
    else if ( syntax == type_string<double>::ident() )
        return Field::Ptr(new Unknown<double>());
    else if ( syntax == type_string<float>::ident() )
        return Field::Ptr(new Unknown<float>());
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
        return FieldCreator( *element, traits ).get_result();
    else if ( element->Value() == std::string("localizations") )
        return Ptr( new ChildrenField( *element, traits ) );
    else
        return Ptr(NULL);
}

struct LocalizationFieldCreator {
    typedef void result_type;

    template <typename Tag>
    void operator()( const Field::Traits& traits, Field::Fields& result, Tag tag ) {
        if (traits.field(tag).is_given) {
            result.push_back(new LocalizationField<Tag>());
        }
    }
};

void create_localization_fields( const Field::Traits& traits, Field::Fields& result )
{
    boost::mpl::for_each<localization::Fields>(boost::bind(
        LocalizationFieldCreator(),
        boost::ref(traits),
        boost::ref(result),
        _1));
}

std::auto_ptr<Field> Field::construct_xyztI( const Field::Traits& traits )
{
    std::auto_ptr<ChildrenField> rv( new ChildrenField(traits) );
    rv->add_field( new LocalizationField< localization::PositionX >() );
    rv->add_field( new LocalizationField< localization::PositionY >() );
    if ( traits.position_z().is_given )
        rv->add_field( new LocalizationField< localization::PositionZ >() );
    rv->add_field( new LocalizationField< localization::ImageNumber >() );
    rv->add_field( new LocalizationField< localization::Amplitude >() );
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

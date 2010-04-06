#ifndef DSTORM_LOCFILE_FIELDS_IMPL_H
#define DSTORM_LOCFILE_FIELDS_IMPL_H

#include "fields.h"
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/output/Traits.h>
#include <boost/units/io.hpp>

namespace dStorm {
namespace LocalizationFile {
namespace field {

template <typename Derived>
std::istream& operator>>(std::istream& from, Eigen::MatrixBase<Derived>& to) {
    for (int r = 0; r < to.rows(); r++)
      for (int c = 0; c < to.cols(); c++)
        from >> to(r,c);
    return from;
}

/* For this module, ignore units in input, just read numbers */
template <typename Unit, typename Value>
std::istream& operator>>(std::istream& from, boost::units::quantity<Unit,Value>& to) {
    Value val;
    from >> val;
    to = boost::units::quantity<Unit,Value>::from_value(val);
    return from;
}

template <typename Attribute>
static void condAddAttribute( 
    XMLNode& node, const Attribute& a, 
    const std::string& n
) {
    if ( a.is_set() ) {
        std::stringstream ss;
        ss << *a;
        node.addAttribute(n.c_str(), ss.str().c_str()); 
    }
}

template <typename Quantity>
static void 
cond_parse_attribute(
    const XMLNode& node,
    const std::string& name,
    simparm::optional<Quantity>& field
) {
    const char* a = node.getAttribute( name.c_str() );
    if ( a != NULL ) {
        Quantity val;
        std::string unit;
        std::stringstream ss( a );
        ss >> val >> unit;
        field = val;
    }
}

inline const std::string
read_attribute(
    const XMLNode& node, 
    const std::string& name
)
{
    const char *a = node.getAttribute(name.c_str());
    if ( a == NULL )
        throw std::runtime_error("Required attribute "
            + name + " not found for entry in " 
            "localization file.");
    else
        return std::string(a);
}

template <typename Prop>
Known<Prop>::Known( const XMLNode& node )
{
    const std::string 
        syntax = read_attribute(node, "syntax"),
        semantic = read_attribute(node, "semantic"),
        unit = read_attribute(node, "unit"),
        expected_unit = 
                boost::units::name_string(
                    typename Prop::ValueUnit());

    if ( syntax != type_string<Value>::ident() )
        throw std::runtime_error("Unexpected syntax field "
            "in localization file: " + syntax );
    if ( semantic != Prop::semantic )
        throw std::logic_error("Parsing wrong field. "
            "Expected semantic " + Prop::semantic +
            ", got " + semantic);
    if ( unit != expected_unit )
        throw std::runtime_error("Unexpected unit in "
            "parsing localization file: Expected " +
            expected_unit + ", but found " + unit);

    if ( Prop::hasMinField )
        cond_parse_attribute( node, "min", minimum );
    if ( Prop::hasMaxField )
        cond_parse_attribute( node, "max", maximum );
}

template <typename Prop>
Known<Prop>::Known( const output::Traits& traits )
{
    output::Traits& t = const_cast<output::Traits&>(traits);
    if ( Prop::hasMinField )
        minimum = Prop::minField(t);
    if ( Prop::hasMaxField )
        maximum = Prop::maxField(t);
}

template <typename Prop>
KnownWithResolution<Prop>::KnownWithResolution
    ( const output::Traits& traits )
: Known<Prop>(traits)
{
    output::Traits& t = const_cast<output::Traits&>(traits);
    if ( Prop::hasResolutionField )
        resolution = Prop::resolutionField(t);
}

template <typename Prop>
KnownWithResolution<Prop>::KnownWithResolution( const XMLNode& node )
: Known<Prop>(node)
{
    if ( Prop::hasResolutionField )
        cond_parse_attribute( node, "resolution", 
                              resolution );
}

template <typename Prop>
XMLNode Known<Prop>::makeNode( XMLNode& top_node )
{
    XMLNode rv = top_node.addChild("field");
    rv.addAttribute("syntax", 
        type_string<Value>::ident().c_str());
    rv.addAttribute("semantic", Prop::semantic.c_str());
    rv.addAttribute("unit", 
        boost::units::name_string(
            typename Prop::ValueUnit()).c_str() );
    condAddAttribute( rv, minimum, "min" );
    condAddAttribute( rv, maximum, "max" );
            
    return rv;
}

template <typename Prop>
XMLNode KnownWithResolution<Prop>::makeNode( XMLNode& top_node )
{
    XMLNode n = Known<Prop>::makeNode( top_node );
    condAddAttribute( n, resolution, "resolution" );
    return n;
}

template <typename Prop>
void Known<Prop>::getTraits(
    input::Traits<Localization>& t ) const
{
    if ( Prop::hasMinField && minimum.is_set() )
        Prop::minField(t) = *minimum;
    if ( Prop::hasMaxField && maximum.is_set() )
        Prop::maxField(t) = *maximum;
}

template <typename Prop>
void KnownWithResolution<Prop>::getTraits(
    input::Traits<Localization>& t ) const
{
    Known<Prop>::getTraits(t);

    if ( Prop::hasResolutionField && resolution.is_set() )
        Prop::resolutionField(t) = *resolution;
}

template <typename Prop>
void Known<Prop>::parse(
    std::istream& input, 
    Localization& target
)
{
    /* TODO: Honor unit given in header here. */
    Value v;
    input >> v;
    Prop::insert( v, target );
}

}
}
}


#endif

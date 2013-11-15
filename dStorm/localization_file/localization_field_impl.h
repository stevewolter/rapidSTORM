#ifndef DSTORM_LOCFILE_FIELDS_IMPL_H
#define DSTORM_LOCFILE_FIELDS_IMPL_H

#include "debug.h"
#include "localization_field.h"
#include <dStorm/localization/Traits.h>
#include <dStorm/output/Traits.h>
#include <boost/units/io.hpp>
#include <iomanip>
#include <sstream>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/at.hpp>

namespace dStorm {
namespace localization_file {

template <typename Attribute>
static void condAddAttribute( 
    TiXmlElement& node, const Attribute& a, 
    const std::string& n
) {
    if ( a.is_initialized() ) {
        std::stringstream ss;
        ss << *a;
        node.SetAttribute(n.c_str(), ss.str().c_str()); 
    }
}

template <typename Quantity>
static void 
cond_parse_attribute(
    const TiXmlElement& node,
    const std::string& name,
    boost::optional<Quantity>& field
) {
    const char* a = node.Attribute( name.c_str() );
    if ( a != NULL ) {
        typename Quantity::value_type val;
        std::string unit;
        std::stringstream ss( a );
        ss >> val >> unit;
        field = Quantity::from_value(val);
    }
}

inline const std::string
read_attribute(
    const TiXmlElement& node, 
    const std::string& name
)
{
    const char *a = node.Attribute(name.c_str());
    if ( a == NULL )
        throw std::runtime_error("Required attribute "
            + name + " not found for entry in " 
            "localization file.");
    else
        return std::string(a);
}

std::string field_identifier(localization::PositionX) { return "Position-0-0"; }
std::string field_identifier(localization::PositionUncertaintyX) { return "Position-0-0-uncertainty"; }
std::string field_identifier(localization::PositionY) { return "Position-1-0"; }
std::string field_identifier(localization::PositionUncertaintyY) { return "Position-1-0-uncertainty"; }
std::string field_identifier(localization::PositionZ) { return "Position-2-0"; }
std::string field_identifier(localization::PositionUncertaintyZ) { return "Position-2-0-uncertainty"; }
std::string field_identifier(localization::ImageNumber) { return "ImageNumber-0-0"; }
std::string field_identifier(localization::Amplitude) { return "Amplitude-0-0"; }
std::string field_identifier(localization::PSFWidthX) { return "PSFWidth-0-0"; }
std::string field_identifier(localization::PSFWidthY) { return "PSFWidth-1-0"; }
std::string field_identifier(localization::TwoKernelImprovement) { return "TwoKernelImprovement-0-0"; }
std::string field_identifier(localization::LocalBackground) { return "LocalBackground-0-0"; }
std::string field_identifier(localization::FitResidues) { return "FitResidues-0-0"; }
std::string field_identifier(localization::Fluorophore) { return "Fluorophore-0-0"; }

template <typename Unit, typename Value>
inline std::string name_string( const quantity<Unit,Value>& );

template <typename Type>
inline std::string name_string( const Eigen::MatrixBase<Type>& ) 
    { return name_string( typename Eigen::MatrixBase<Type>::Scalar() ); }

template <typename Unit, typename Value>
inline std::string name_string( const quantity<Unit,Value>& ) 
    { return name_string( Unit() ); }

inline std::string name_string( int ) 
    { return name_string( si::dimensionless() ); }
inline std::string name_string( float ) 
    { return name_string( si::dimensionless() ); }
inline std::string name_string( double ) 
    { return name_string( si::dimensionless() ); }

static std::string guess_ident_from_semantic(const TiXmlElement& n) {
    std::string semantic;
    const char *semantic_attrib = n.Attribute("semantic");
    if ( semantic_attrib != NULL ) semantic = semantic_attrib;
    if ( semantic == "x-position" )
        return field_identifier(localization::PositionX());
    else if ( semantic == "y-position" )
        return field_identifier(localization::PositionY());
    else if ( semantic == "z-position" )
        return field_identifier(localization::PositionZ());
    else if ( semantic == "frame number" )
        return field_identifier(localization::ImageNumber());
    else if ( semantic == "emission strength" )
        return field_identifier(localization::Amplitude());
    else if ( semantic == "two kernel improvement" )
        return field_identifier(localization::TwoKernelImprovement());

    // TODO: Implement more heuristics: x-sigma i.e.
    return "";
}

inline void output_value_only( std::ostream& o, float a ) { o << a; }
inline void output_value_only( std::ostream& o, double a ) { o << a; }

template <typename Unit, typename Value>
inline void output_value_only( std::ostream& o, quantity<Unit,Value> a )
{
    o << a.value();
}

template <typename Tag>
std::string LocalizationField<Tag>::ident_field(const TiXmlElement& n)
{
        std::string ident;
        if ( n.Attribute("identifier") )
            ident = n.Attribute("identifier");
        if ( ident == "" ) {
            ident = guess_ident_from_semantic(n);
        }
        if ( ident == "" )
            throw std::runtime_error("Identifier field is missing and could not guess from semantic field '" + std::string(n.Attribute("semantic")) + "'");
        else
            return ident;
    }

template <typename Tag>
std::string LocalizationField<Tag>::dimen_name(int d) {
    switch (d) {
        case 0: return "x";
        case 1: return "y";
        case 2: return "z";
        case 3: return "a";
        default:
            throw std::logic_error("Ran out of dimension names");
    }
}

template <typename Tag>
Field::Ptr LocalizationField<Tag>::try_to_parse( const TiXmlElement& n, TraitsType& traits ) 
{
    std::string ident = ident_field(n);

    if ( ident == field_identifier(Tag()) )
        return Field::Ptr( new LocalizationField<Tag>(n, traits) );

    return Field::Ptr();
}

template <typename Tag>
bool has_implied_lower_boundary_of_zero(Tag tag) {
    return false;
}

template <int Dimension>
bool has_implied_lower_boundary_of_zero(localization::Position<Dimension> tag) {
    return true;
}

template <typename Tag>
LocalizationField<Tag>::LocalizationField( const TiXmlElement& node, TraitsType& localization_traits ) 
{
    TraitsType& traits = localization_traits;
    const std::string 
        syntax = read_attribute(node, "syntax");

    if ( syntax != type_string<typename Tag::ValueType>::ident() )
        throw std::runtime_error("Unrecognized syntax "
            "in localization file: " + syntax );

    traits.is_given = true;

    if ( TraitsType::has_range ) {
        /* Backward compatibility: Old versions of the XML syntax didn't require lower boundaries for the
          * spatial coordinates, but implied 0. */
        if ( has_implied_lower_boundary_of_zero(Tag())
             && node.Attribute("min") == NULL && node.Attribute("identifier")  == NULL ) {
            DEBUG("Setting field minimum to 0");
            traits.range().first = ValueType::from_value(0);
        }
        cond_parse_attribute( node, "min", traits.range().first );
        cond_parse_attribute( node, "max", traits.range().second );
    }

    set_input_unit( read_attribute(node, "unit"), localization_traits );
}

template <typename Tag>
LocalizationField<Tag>::LocalizationField() {}

template <typename Tag>
LocalizationField<Tag>::~LocalizationField()  {}

template <typename Tag>
std::auto_ptr<TiXmlNode> LocalizationField<Tag>::makeNode( const Field::Traits& traits ) { 
    std::auto_ptr<TiXmlElement> rv( new  TiXmlElement("field") );
    rv->SetAttribute( "identifier", field_identifier(Tag()).c_str() );
    rv->SetAttribute( "syntax", type_string< typename Tag::ValueType >::ident().c_str() );

    std::stringstream semantic;
    semantic << Tag::get_desc();
    rv->SetAttribute( "semantic", semantic.str().c_str());
    rv->SetAttribute( "unit",
        name_string(
            typename Tag::OutputType()).c_str() );
    
    if ( TraitsType::has_range ) {
        condAddAttribute( *rv, traits.field(Tag()).range().first, "min" );
        condAddAttribute( *rv, traits.field(Tag()).range().second, "max" );
    }

    return std::auto_ptr<TiXmlNode>( rv.release() );
}

template <typename Tag>
void LocalizationField<Tag>::set_input_unit(const std::string& unit, const Field::Traits& traits)
{
    set_input_unit( unit, traits );
}

template <typename Tag>
void LocalizationField<Tag>::set_input_unit(const std::string& unit, const TraitsType& traits)
{
    converter = Converter::create( unit );
    if ( converter.get() == NULL )
        throw std::runtime_error("Unrecognized unit in "
            "parsing localization file: " + unit);
}

template <typename Tag>
void LocalizationField<Tag>::write(std::ostream& output, const Localization& source)
{
    typename Tag::OutputType ov 
        = static_cast<typename Tag::OutputType>( source.field(Tag()).value() );
    output_value_only(output, ov);
}

template <typename Tag>
void LocalizationField<Tag>::parse(std::istream& input, Localization& target)
{
    typename ValueType::value_type v;
    input >> v;
    target.field(Tag()).value() = converter->from_value(v);
}

template <typename Type, typename LocalizationField>
struct NodeMaker
{
    static boost::ptr_vector<Field> make_nodes(const typename LocalizationField::TraitsType& traits) {
        boost::ptr_vector<Field> rv;
        if ( traits.is_given )
            rv.push_back( new LocalizationField() );
        return rv;
    }
};

}
}


#endif

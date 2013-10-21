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
        return LocalizationField<Localization::Fields::Position>::identifier(0,0);
    else if ( semantic == "y-position" )
        return LocalizationField<Localization::Fields::Position>::identifier(1,0);
    else if ( semantic == "z-position" )
        return LocalizationField<Localization::Fields::Position>::identifier(2,0);
    else if ( semantic == "frame number" )
        return LocalizationField<Localization::Fields::ImageNumber>::identifier(0,0);
    else if ( semantic == "emission strength" )
        return LocalizationField<Localization::Fields::Amplitude>::identifier(0,0);
    else if ( semantic == "two kernel improvement" )
        return LocalizationField<Localization::Fields::TwoKernelImprovement>::identifier(0,0);

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

template <int Index>
std::string LocalizationField<Index>::identifier(int r, int c)
{
    std::stringstream ident;
    ident << TraitsType::get_ident() << "-" << r << "-" << c;
    return ident.str();
}

template <int Index>
std::string LocalizationField<Index>::ident_field(const TiXmlElement& n)
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

template <int Index>
std::string LocalizationField<Index>::dimen_name(int d) {
    switch (d) {
        case 0: return "x";
        case 1: return "y";
        case 2: return "z";
        case 3: return "a";
        default:
            throw std::logic_error("Ran out of dimension names");
    }
}

template <int Index>
Field::Ptr LocalizationField<Index>::try_to_parse( const TiXmlElement& n, TraitsType& traits ) 
{
    std::string ident = ident_field(n);

    if ( ident == TraitsType::get_ident() )
        return Field::Ptr( new LocalizationField<Index>(n, traits) );

    for (int r = 0; r < TraitsType::Rows; ++r)
        for (int c = 0; c < TraitsType::Cols; ++c)
            if ( identifier(r,c) == ident )
                return Field::Ptr( new LocalizationField<Index>(n, traits, r, c) );

    return Field::Ptr();
}

template <int Index>
LocalizationField<Index>::LocalizationField( const TiXmlElement& node, TraitsType& traits, int row, int column ) 
: scalar(row, column)
{
    const std::string 
        syntax = read_attribute(node, "syntax");

    if ( syntax != type_string<typename TraitsType::ValueType>::ident() )
        throw std::runtime_error("Unrecognized syntax "
            "in localization file: " + syntax );

    scalar.is_given(traits) = true;

    if ( TraitsType::has_range ) {
        /* Backward compatibility: Old versions of the XML syntax didn't require lower boundaries for the
          * spatial coordinates, but implied 0. */
        if ( Index == Localization::Fields::Position && row < 2 && column == 0 && node.Attribute("min") == NULL && node.Attribute("identifier")  == NULL ) {
            DEBUG("Setting field minimum to 0");
            scalar.range(traits).first = Scalar::range_type::first_type::value_type::from_value(0);
        }
        cond_parse_attribute( node, "min", scalar.range(traits).first );
        cond_parse_attribute( node, "max", scalar.range(traits).second );
    }

    set_input_unit( read_attribute(node, "unit"), traits );
}

template <int Index>
LocalizationField<Index>::LocalizationField(int row, int column) 
: scalar(row, column) {}

template <int Index>
LocalizationField<Index>::~LocalizationField()  {}

template <int Index>
std::auto_ptr<TiXmlNode> LocalizationField<Index>::makeNode( const Field::Traits& traits ) { 
    std::auto_ptr<TiXmlElement> rv( new  TiXmlElement("field") );
    rv->SetAttribute( "identifier", identifier(scalar.row(),scalar.column()).c_str() );
    rv->SetAttribute( "syntax", type_string< typename TraitsType::ValueType >::ident().c_str() );

    std::stringstream semantic;
    semantic << TraitsType::get_desc();
    if ( TraitsType::Rows > 1 || TraitsType::Cols > 1 ) {
        semantic << " in ";
        if ( TraitsType::Rows > 1 ) semantic << dimen_name(scalar.row());
        if ( TraitsType::Cols > 1 ) semantic << dimen_name(scalar.column());
        semantic << " dimension";
    }
    rv->SetAttribute( "semantic", semantic.str().c_str());
    rv->SetAttribute( "unit",
        name_string(
            typename TraitsType::OutputType()).c_str() );
    
    if ( TraitsType::has_range ) {
        condAddAttribute( *rv, scalar.range(traits).first, "min" );
        condAddAttribute( *rv, scalar.range(traits).second, "max" );
    }

    return std::auto_ptr<TiXmlNode>( rv.release() );
}

template <int Index>
void LocalizationField<Index>::set_input_unit(const std::string& unit, const Field::Traits& traits)
{
    set_input_unit( unit, traits );
}

template <int Index>
void LocalizationField<Index>::set_input_unit(const std::string& unit, const TraitsType& traits)
{
    converter = Converter::create( unit );
    if ( converter.get() == NULL )
        throw std::runtime_error("Unrecognized unit in "
            "parsing localization file: " + unit);
}

template <int Index>
void LocalizationField<Index>::write(std::ostream& output, const Localization& source)
{
    const typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Index> >::type&
        field = boost::fusion::at_c<Index>(source);

    typename TraitsType::OutputType ov 
        = static_cast<typename TraitsType::OutputType>( scalar.value( field.value() ) );
    output_value_only(output, ov);
}

template <int Index>
void LocalizationField<Index>::parse(std::istream& input, Localization& target)
{
    typename Scalar::value_type::value_type v;
    input >> v;
    typename Scalar::value_type t = converter->from_value(v);
    scalar.value( boost::fusion::at_c<Index>(target).value() ) = t;
}

template <typename Type, typename LocalizationField>
struct NodeMaker
{
    static boost::ptr_vector<Field> make_nodes(const typename LocalizationField::TraitsType& traits) {
        boost::ptr_vector<Field> rv;
        if ( traits.is_given )
            rv.push_back( new LocalizationField( 0, 0 ) );
        return rv;
    }
};

template <typename Scalar, int Rows, int Cols, int Flags, int MaxRows, int MaxCols, typename LocalizationField>
struct NodeMaker< Eigen::Matrix<Scalar, Rows, Cols, Flags, MaxRows, MaxCols>, LocalizationField >
{
    static boost::ptr_vector<Field> make_nodes(const typename LocalizationField::TraitsType& traits) {
        boost::ptr_vector<Field> rv;
        for (int r = 0; r < traits.is_given.rows(); ++r)
          for (int c = 0; c < traits.is_given.cols(); ++c)
          {
            if ( traits.is_given(r,c) )
                rv.push_back( new LocalizationField( r, c ) );
          }
        return rv;
    }
};

template <int Index>
boost::ptr_vector<Field> LocalizationField<Index>::make_nodes(const TraitsType& traits)
{
    return NodeMaker<typename TraitsType::ValueType, LocalizationField<Index> >::make_nodes(traits);
}

template <int Index>
Field::Ptr create_localization_field( int row, int column )
{
    return Field::Ptr(new LocalizationField<Index>(row, column));
}

}
}


#endif

#include "dStorm/localization_file/children_field.h"

#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "dStorm/localization/Traits.h"

namespace dStorm {
namespace localization_file {

void ChildrenField::write(std::ostream& output, const Localization& source) {
    for (size_t i = 0; i < attributes.size(); ++i) {
        if ( i != 0 ) output << " ";
        attributes[i].write( output, source );
    }
    if ( children.size() <= 0 )  return;
    std::vector<Localization>::const_iterator 
        c = source.children->begin(),
        e = source.children->end();
    for (size_t i = 0; i < children.size(); ++i) {
        boost::optional<int> max_repetitions = children[i].get_repetition_count();
        if ( ! max_repetitions.is_initialized() ) {
            output << " " << (e-c);
            while ( c != e ) {
                output << " ";
                children[i].write( output, *c++ );
            }
        } else {
            for (int k = 0; k < *max_repetitions; ++k) {
                output << " ";
                assert( c != e );
                children[i].write( output, *c++ );
            }
        }
    }
}

void ChildrenField::parse(std::istream& input, Localization& upper_target) {
    if ( ! upper_target.children.is_initialized() )
        upper_target.children = std::vector<Localization>();
    for (int repetition = read_repetition_count(input); repetition > 0; --repetition) {
        upper_target.children->push_back( Localization() );
        Localization& target = upper_target.children->back();
        for (size_t i = 0; i < attributes.size(); ++i)
            attributes[i].parse( input, target );
        for (size_t i = 0; i < children.size(); ++i) {
            children[i].parse( input, target );
        }
    }
}

std::auto_ptr<TiXmlNode> ChildrenField::makeNode( const Traits& traits ) {
    std::auto_ptr<TiXmlElement> rv( new TiXmlElement("localizations") );
    rv->SetAttribute("insequence", (traits.in_sequence) ? "true" : "false");
    std::string children_count_value =
        ( traits.repetitions.is_initialized() )
            ? boost::lexical_cast<std::string,int>( *traits.repetitions )
            : "variable";
    rv->SetAttribute("repetitions", children_count_value.c_str());
    BOOST_FOREACH( Field& i, attributes )
        rv->LinkEndChild( i.makeNode( traits ).release() );
    for (int i = 0; i < int( children.size() ); ++i)
        rv->LinkEndChild( children[i].makeNode( *traits.source_traits[i] ).release() );
    return std::auto_ptr<TiXmlNode>( rv.release() );
}

ChildrenField::ChildrenField( const Traits& traits)
: separator( "\n" ), repetitions( traits.repetitions )
{}

ChildrenField::ChildrenField( const Traits& traits, int level )
: separator( level > 0 ? " " : "\n" ), repetitions( traits.repetitions )
{
    create_localization_fields( traits, attributes );
    for (size_t i = 0; i < traits.source_traits.size(); ++i)
        children.push_back( new ChildrenField( *traits.source_traits[i], level + 1 ) );
}

ChildrenField::ChildrenField( const TiXmlElement& node, Traits& traits ) 
: separator("")
{
    boost::shared_ptr<Traits> t( new Traits() );
    t->in_sequence = 
        node.Attribute("insequence") && node.Attribute("insequence") == std::string("true");
    if ( ! node.Attribute("repetitions") )
        t->repetitions = 1;
    else if ( node.Attribute("repetitions") == std::string("variable") )
        t->repetitions.reset();
    else
        t->repetitions = boost::lexical_cast<int>( std::string(node.Attribute("repetitions")) );
    repetitions = t->repetitions;
        
    for (const TiXmlNode* child = node.FirstChild(); child; child = child->NextSibling() ) {
        Field::Ptr p = Field::parse(*child, *t);
        if ( dynamic_cast< ChildrenField* >(p.get()) != NULL ) {
            children.push_back( p );
        } else if ( p.get() != NULL )
            attributes.push_back(p);
    }
    traits.source_traits.push_back(t);
}

int ChildrenField::read_repetition_count(std::istream& input)
{
    if ( repetitions.is_initialized() )
        return *repetitions;
    else {
        int rv;
        input >> rv;
        if ( ! input ) return 0;
        return rv;
    }
}

}
}

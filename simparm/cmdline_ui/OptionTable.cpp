#include "OptionTable.h"
#include <simparm/BaseAttribute.h>
#include <boost/lexical_cast.hpp>

namespace simparm {
namespace cmdline_ui {

int OptionTable::parse( int argc, char** args ) {
    if ( argc <= 0 ) return 0;
    std::string name_stem = args[0];
    if ( name_stem.substr(0,2) != "--" ) 
        return 0;
    else
        name_stem = name_stem.substr(2);

    /* Parse the optional number index prefix */
    int index_specification = -1;
    size_t pointpos = name_stem.find('.');
    if ( pointpos != std::string::npos ) {
        try {
            index_specification = boost::lexical_cast<int>( name_stem.substr(0,pointpos) );
            name_stem = name_stem.substr( pointpos + 1 );
        } catch ( boost::bad_lexical_cast ) {
            std::cerr << "Number index prefix unrecognized in " << name_stem << std::endl;
            return 0;
        }
    }

    /* Parse optional no- modificator */
    bool has_no = false;
    if ( name_stem.substr(0,3) == "no-" || name_stem.substr(0,3) == "No-" ) {
        name_stem = name_stem.substr(3);
        has_no = true;
    }

    std::vector<Option>::const_iterator i = 
        std::lower_bound( options.begin(), options.end(), name_stem );

    if ( i == options.end() ) return 0;

    /* Check whether matched name is unique. */
    if ( i->name != name_stem ) {
        std::vector<Option>::const_iterator j = i;
        while ( j != options.end() && j->name.substr( name_stem.length() ) == name_stem )
            ++j;
        if ( j == i ) {
            return 0;
        } else if ( j - i > 1 )  {
            std::cerr << "Option " << name_stem << " is ambiguous, candidates are: ";
            for ( ; i != j ; ++i ) std::cerr << i->name << " ";
            std::cerr << std::endl;
        }
    }

    BaseAttribute* attribute = NULL;
    if ( index_specification == -1 ) 
        attribute = i->attributes.back();
    else if ( index_specification < i->attributes.size() )
        attribute = i->attributes[ index_specification-1 ];
    else
        throw std::runtime_error("Index specification " + boost::lexical_cast<std::string>(index_specification) + " is out of bounds");

    std::string value;
    switch ( i->type ) {
        case Boolean: value = ( has_no ) ? "false" : "true"; break;
        case Value: 
            if ( argc <= 1 ) {
                std::cerr << "Option " << name_stem << " takes a mandatory argument" << std::endl;
                return 0;
            } else {
                value = args[1];
            }
            break;
        case Trigger:
            value = "1";
            break;
    }
    std::stringstream stream(value);
    attribute->set_value("set", stream);
    return 0;
}

void OptionTable::add_option( std::string name, BaseAttribute& a, Type t ) { 
    std::vector<Option>::iterator i = 
        std::lower_bound( options.begin(), options.end(), name );
    if ( i == options.end() || i->name != name ) {
        i = options.insert( i, Option(name) );
    }
    i->add( a, t );
}

}
}

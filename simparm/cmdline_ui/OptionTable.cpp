#include "OptionTable.h"
#include <simparm/BaseAttribute.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace simparm {
namespace cmdline_ui {

bool OptionTable::Option::operator<( const Option& o ) const {
    return boost::ilexicographical_compare( name, o.name );
}

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
    if ( boost::iequals( name_stem.substr(0,3), "no-" ) ) {
        name_stem = name_stem.substr(3);
        has_no = true;
    }

    std::vector<Option>::const_iterator i = 
        std::lower_bound( options.begin(), options.end(), name_stem );

    if ( i == options.end() ) return 0;

    /* Check whether matched name is unique. */
    if ( i->name != name_stem ) {
        std::vector<Option>::const_iterator j = i;
        while ( j != options.end() && boost::iequals( j->name.substr( 0, name_stem.length() ), name_stem ) )
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
    else if ( index_specification-1 < int(i->attributes.size()) )
        attribute = i->attributes[ index_specification-1 ];
    else
        throw std::runtime_error("Index specification " + boost::lexical_cast<std::string>(index_specification) + " is out of bounds");

    int parsed_args = 1;
    std::string value;
    switch ( i->type ) {
        case Boolean: value = ( has_no ) ? "false" : "true"; break;
        case Value: 
            if ( argc <= 1 ) {
                std::cerr << "Option " << name_stem << " takes a mandatory argument" << std::endl;
                return 0;
            } else {
                value = args[1];
                parsed_args = 2;
            }
            break;
        case Trigger:
            value = "1";
            break;
        default:
            throw std::logic_error("Fall-through for command line option type");
    }
    attribute->set_value(value);
    return parsed_args;
}

void OptionTable::add_option( std::string name, std::string desc, std::string help, std::string choices, BaseAttribute& a, Type t ) { 
    std::vector<Option>::iterator i = 
        std::lower_bound( options.begin(), options.end(), name );
    if ( i == options.end() || ! boost::iequals( i->name, name ) ) {
        i = options.insert( i, Option(name) );
    }
    i->add( a, t );
    i->desc = desc;
    i->help = help;
    i->choices = choices;
}

static void formatParagraph(std::ostream &o, unsigned int left_col, 
                   unsigned int right_col, const std::string &s) 
{
   unsigned int pos, lookahead = 0;
   unsigned int cur_col = left_col;
   while (lookahead < s.length()) {
      pos = lookahead;
      if (isalpha(s[lookahead]))
         while (lookahead < s.length() && 
                isalpha(s[lookahead])) lookahead++;
      else
         lookahead++;

      if ((lookahead-pos) > 1+(right_col - cur_col)) {
         o << "\n";
         cur_col = 0;
         while (cur_col < left_col) { cur_col++; o << " "; }
      }
      if (cur_col == left_col && isspace(s[pos]))
         /* skip */;
      else
         o << s.substr(pos, lookahead-pos);
      cur_col += lookahead - pos;
   }
   while (cur_col++ <= right_col) o << " ";
}

void OptionTable::Option::print_help( std::ostream& o ) const {
   std::string n = "--" + name.substr(0, std::min<int>(name.length(), 19));
   formatParagraph(o, 0, 20, n);
   o << "  ";
   formatParagraph(o, 23, 79, desc);
   o << "\n";
   if (help != "") {
      for (int i = 0; i < 23; i++) o << " ";
      formatParagraph(o, 23, 79, help);
      o << "\n";
   }
   if ( choices != "" ) {
        formatParagraph(o, 0, 22, "");
        formatParagraph(o, 23, 79, "Choices are: " + choices);
        o << "\n";
   }
}

void OptionTable::printHelp( std::ostream& o ) {
    std::for_each( options.begin(), options.end(),
        boost::bind( &Option::print_help, _1, boost::ref(o) ) );
}

}
}

#include "debug.h"
#include "Basename.h"
#include <sstream>
#include <stdexcept>
#include <memory>

using namespace std;

namespace dStorm {
namespace output {

Basename::Basename(
    const std::string& base,
    const ReplaceMap& replace )
: basename("value", base),
  replacements(replace)
{
}

Basename::Basename( const string& base )
: basename("value", base) 
{
    replacements.insert( make_pair("", "$") );
}

Basename::Basename( const Basename& base )
: basename(base.basename),
  replacements(base.replacements)
{
}

Basename& Basename::operator=
    ( const Basename& base )
{
    basename = base.basename;
    replacements = base.replacements;
    return *this;
}

Basename Basename::append
    ( const string& suffix )
{
    return Basename( basename() + suffix, replacements );
}

void Basename::set_variable( 
    const string& name,
    const string& value )
{
    replacements.insert( make_pair(name,value) );
}

string Basename::new_basename() const {
    stringstream result;
    std::auto_ptr<stringstream> varname;
    enum State { Free, InVariable };

    State s = Free;
    for (size_t i = 0; i < basename().size(); i++)
    {
        const char c = basename()[i];
        if ( c != '$' )
            ( (s == Free) ? result : *varname ) << c;
        else if ( s == Free ) {
            s = InVariable;
            varname.reset( new std::stringstream() );
        } else {
            string var = varname->str();
            ReplaceMap::const_iterator j = replacements.find(var);
            
            if ( j == replacements.end() ) {
                throw runtime_error("Undefined variable '" + var + "'");
            } else
                result << j->second;
            s = Free;
        }
    }

    DEBUG("Expanded " << basename() << " to " << result.str());
    return result.str();
}

}
}

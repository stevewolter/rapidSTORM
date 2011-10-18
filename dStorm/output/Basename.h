#ifndef DSTORM_OUTPUT_BASENAME_H
#define DSTORM_OUTPUT_BASENAME_H

#include <string>
#include <iostream>
#include <map>
#include <simparm/Callback.hh>
#include <simparm/Attribute.hh>

namespace dStorm {
namespace output {

struct Basename : public simparm::Publisher {
    typedef std::map< std::string, std::string > ReplaceMap;
  private:
    simparm::Attribute<std::string> basename;
    ReplaceMap replacements;

    Basename( const std::string& base,
              const ReplaceMap& replace );
  public:
    Basename( const std::string& base = "" );
    Basename( const Basename& );
    Basename& operator=( const Basename& );

    simparm::Attribute<std::string>& 
        unformatted() { return basename; }
    const simparm::Attribute<std::string>& 
        unformatted() const { return basename; }
    Basename append( const std::string& );
    void set_variable( 
        const std::string& name,
        const std::string& value );

    std::string new_basename() const;

    const ReplaceMap& replacement_map() const { return replacements; }
};

std::ostream& operator<<( std::ostream&, const Basename& );

}
}

#endif

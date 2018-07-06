#ifndef DSTORM_localization_file_unknown_field_h
#define DSTORM_localization_file_unknown_field_h

#include "localization_file/field.h"

namespace dStorm {
namespace localization_file {

template <typename Type>
class Unknown : public Field {
    void parse(std::istream& input, Localization&) 
        { Type ignore; input >> ignore; }
    void write(std::ostream&, const Localization&) {}
    Unknown<Type>* clone() const
        { return new Unknown<Type>(); }
    std::auto_ptr<TiXmlNode> makeNode( const Traits& ) { return std::auto_ptr<TiXmlNode>(); }
};

}
}

#endif

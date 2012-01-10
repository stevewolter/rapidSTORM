#ifndef DSTORM_LOCALIZATION_FILE_CHILDREN_FIELD_H
#define DSTORM_LOCALIZATION_FILE_CHILDREN_FIELD_H

#include "field.h"

namespace dStorm {
namespace localization_file {

struct ChildrenField : public Field {
    Fields attributes, children;
    const std::string separator;
    boost::optional<int> repetitions;

    void write(std::ostream& output, const Localization& source);
    void parse(std::istream& input, Localization& target);
    std::auto_ptr<TiXmlNode> makeNode( const Traits& traits );
    int read_repetition_count(std::istream&);
    boost::optional<int> get_repetition_count() { return repetitions; }

    Field* clone() const { return new ChildrenField(*this); }
  public:
    ChildrenField( const Traits& traits );
    ChildrenField( const Traits& traits, int level );
    ChildrenField( const TiXmlElement&, Traits& traits );

    void add_field( std::auto_ptr<Field> f )
        { attributes.push_back( f ); }
    void add_field( Field* f ) { add_field( std::auto_ptr<Field>(f) ); }
};

}
}

#endif

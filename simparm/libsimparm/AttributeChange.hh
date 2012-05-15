#ifndef SIMPARM_ATTRIBUTECHANGE_H
#define SIMPARM_ATTRIBUTECHANGE_H

#include "Attribute.hh"

namespace simparm {

template <typename AttributeType>
class AttributeChange {
    simparm::Attribute<AttributeType>* attribute;
    AttributeType value_before;

  public:
    AttributeChange(simparm::Attribute<AttributeType>& a, 
                    const AttributeType& new_value)
    : attribute(&a), value_before(a())
    {
        (*attribute) = new_value;
    }

    void release() {  attribute = NULL; }

    ~AttributeChange() { if ( attribute) (*attribute) = value_before; }
};

}

#endif

#ifndef DSTORM_EXPRESSION_VARIABLE_H
#define DSTORM_EXPRESSION_VARIABLE_H

#include "expression/DynamicQuantity.h"
#include "localization/Traits_decl.h"
#include "Localization_decl.h"

namespace dStorm {
namespace expression {

class Variable {
  public:
    std::string name;

    Variable( const std::string& name ) : name(name) {}
    virtual ~Variable() {}
    virtual Variable* clone() const = 0;
    virtual bool is_static( const input::Traits<Localization>& ) const = 0;
    virtual DynamicQuantity get( const input::Traits<Localization>& ) const = 0;
    virtual DynamicQuantity get( const Localization& ) const = 0;
    virtual void set( input::Traits<Localization>&, const DynamicQuantity& ) const = 0;
    virtual bool set( const input::Traits<Localization>&, Localization&, const DynamicQuantity& ) const = 0;
};

}
}

#endif

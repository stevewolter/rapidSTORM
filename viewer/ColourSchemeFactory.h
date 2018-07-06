#ifndef DSTORM_VIEWER_COLOURSCHEMEFACTORY_H
#define DSTORM_VIEWER_COLOURSCHEMEFACTORY_H

#include "simparm/ObjectChoice.h"
#include "simparm/BaseAttribute.h"
#include <memory>
#include "viewer/Backend.h"

namespace dStorm {
namespace viewer {

class ColourScheme;

class ColourSchemeFactory : public simparm::ObjectChoice {
  public:
    ColourSchemeFactory( std::string name, std::string desc )
        : simparm::ObjectChoice(name,desc) {}
    virtual ColourSchemeFactory* clone() const = 0;

    virtual std::auto_ptr<ColourScheme> make_backend( bool invert ) const = 0; 
    virtual void add_listener( simparm::BaseAttribute::Listener ) = 0;
};

}
}

#endif

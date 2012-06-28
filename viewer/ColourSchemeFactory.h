#ifndef DSTORM_VIEWER_COLOURSCHEMEFACTORY_H
#define DSTORM_VIEWER_COLOURSCHEMEFACTORY_H

#include <simparm/ObjectChoice.h>
#include <simparm/BaseAttribute.h>
#include <memory>
#include "Backend.h"
#include <dStorm/make_clone_allocator.hpp>

namespace dStorm {
namespace viewer {
namespace colour_schemes { class Base; }

struct ColourSchemeFactory : public simparm::ObjectChoice {
    ColourSchemeFactory( std::string name, std::string desc )
        : simparm::ObjectChoice(name,desc) {}
    virtual ColourSchemeFactory* clone() const = 0;

    virtual std::auto_ptr<colour_schemes::Base> make_backend( bool invert ) const = 0; 
    virtual void add_listener( simparm::BaseAttribute::Listener ) = 0;
};

}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::viewer::ColourSchemeFactory)

#endif

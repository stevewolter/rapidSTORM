#ifndef ANDORCAMERA_CONTEXT_H
#define ANDORCAMERA_CONTEXT_H

#include <dStorm/input/Context.h>
#include <boost/shared_ptr.hpp>

namespace AndorCamera {

struct Context : public dStorm::input::Context {
    typedef boost::shared_ptr<Context> Ptr;

    Context() : default_to_live_view(NULL) {}
    Context( const dStorm::input::Context& c,
                   bool default_to_live_view )
        : dStorm::input::Context(c), 
          default_to_live_view(default_to_live_view) {}
    Context* clone() const { return new Context(*this); }

    bool default_to_live_view;
};

}

#endif

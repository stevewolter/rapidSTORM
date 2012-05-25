#ifndef DSTORM_SPOTFINDER_BUILDER_H
#define DSTORM_SPOTFINDER_BUILDER_H

#include "SpotFinder.h"
#include <simparm/Object.h>

namespace dStorm {
namespace engine {
namespace spot_finder {

    template <typename Config, typename SpotFinder>
    class Builder : public Factory
    {
        Config config;
        simparm::Object name_object;
    public:
        Builder() : name_object( Config::get_name(), Config::get_description() ) {}
        Builder* clone() const 
            { return new Builder(*this); }
        std::auto_ptr<Base> make(const Job& job) const
            { return std::auto_ptr<Base>( new SpotFinder( config, job ) ); }
        void attach_ui( simparm::NodeHandle to ) { 
            config.attach_ui( name_object.attach_ui(to) );
        }
        void detach_ui( simparm::NodeHandle to ) { name_object.detach_ui(to); }
        std::string getName() const { return Config::get_name(); }
    };

}
}
}

#endif

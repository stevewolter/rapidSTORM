#include "inputs/YMirror.h"

#include "engine/Image.h"
#include "helpers/make_unique.hpp"
#include "image/Image.h"
#include "image/mirror.h"
#include "input/AdapterSource.h"
#include "input/FilterFactory.h"
#include "simparm/Entry.h"
#include "simparm/Object.h"

namespace dStorm {
namespace YMirror {

struct Config
{
    simparm::Object name_object;
    simparm::BoolEntry mirror_y;
    Config();
    void attach_ui( simparm::NodeHandle at ) { mirror_y.attach_ui( name_object.attach_ui(at) ); }
};

class Source
: public input::AdapterSource< engine::ImageStack >,
  boost::noncopyable
{
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE;

  public:
    Source( std::unique_ptr<input::Source<engine::ImageStack>> base )
        : input::AdapterSource<engine::ImageStack>(std::move(base)) {}
};

bool Source::GetNext(int thread, engine::ImageStack* target) {
    if (!input::AdapterSource<engine::ImageStack>::GetNext(thread, target)) {
        return false;
    }

    for (int i = 0; i < target->plane_count(); ++i) {
        target->plane(i) = image::mirror(target->plane(i), 1);
    }

    return true;
}

class ChainLink
: public input::FilterFactory<engine::ImageStack>
{
    boost::shared_ptr<const input::Traits<engine::ImageStack>> make_meta_info(
        input::MetaInfo& meta_info,
        boost::shared_ptr<const input::Traits<engine::ImageStack>> input_meta_info)
        OVERRIDE {
        return input_meta_info;
    }
    std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) OVERRIDE {
        if ( config.mirror_y() )
            return make_unique<Source>(std::move(input));
        else
            return std::move(input);
    }

    Config config;
  public:
    ChainLink() {}
    ChainLink* clone() const OVERRIDE { return new ChainLink(); }

    void attach_ui(simparm::NodeHandle at,
                   std::function<void()> traits_change_callback) OVERRIDE {
        config.attach_ui( at ); 
    }
};

Config::Config() 
: name_object( "Mirror", "Mirror input data along Y axis"),
  mirror_y("MirrorY", false)
{
    mirror_y.set_user_level( simparm::Expert );
}

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create() {
    return make_unique<ChainLink>();
}

}
}

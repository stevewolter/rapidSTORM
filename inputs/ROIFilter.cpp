#include "inputs/ROIFilter.h"

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

#include "helpers/make_unique.hpp"
#include "image/extend.h"
#include "image/Image.h"
#include "image/slice.h"
#include "input/AdapterSource.h"
#include "input/FilterFactory.h"
#include "input/InputMutex.h"
#include "localization/Traits.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"
#include "simparm/BoostUnits.h"
#include "UnitEntries/FrameEntry.h"
#include "units/frame_count.h"

namespace dStorm {
namespace ROIFilter {

struct Config {
    simparm::Object name_object;
    IntFrameEntry first_frame;
    simparm::Entry< boost::optional< frame_index > > last_frame;

    Config();
    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = name_object.attach_ui(at);
        first_frame.attach_ui( r ); 
        last_frame.attach_ui( r ); 
    }
};

struct ImageNumber
: public boost::static_visitor<frame_index>
{
    template <typename Type>
    frame_index operator()( const Type& e )
        { return e.frame_number(); }
};

class Source
: public input::AdapterSource<engine::ImageStack> {
    const frame_index from;
    const boost::optional<frame_index> to;
    bool saw_last_image;

    inline bool is_in_range(const engine::ImageStack& t) const;
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE;

  public:
    Source( std::unique_ptr< input::Source<engine::ImageStack> > upstream,
            frame_index from, boost::optional<frame_index> to)
        : input::AdapterSource<engine::ImageStack>(std::move(upstream)), from(from), to(to), saw_last_image(false) {}

    void modify_traits( input::Traits<engine::ImageStack>& p) {
        frame_index from = std::max( this->from, *p.image_number().range().first );
        if ( p.image_number().range().second.is_initialized() && to.is_initialized() ) {
            p.image_number().range().second = std::min(*to, *p.image_number().range().second);
        } else if ( to.is_initialized() ) {
            p.image_number().range().second = *to;
        }
        p.image_number().range().first = from;
    }
};

bool Source::GetNext(int thread, engine::ImageStack* target) {
    if (saw_last_image) {
        return false;
    }

    while (true) {
        if (!input::AdapterSource<engine::ImageStack>::GetNext(thread, target)) {
            return false;
        }

        if (to && ImageNumber()(*target) == *to) {
            saw_last_image = true;
        }

        if (is_in_range(*target)) {
            return true;
        }
    }
}

bool Source::is_in_range(const engine::ImageStack& t) const {
    frame_index f = ImageNumber()(t);
    bool rv = f >= from && (!to.is_initialized() || f <= *to);
    return rv;
}

class FilterFactory 
: public input::FilterFactory<engine::ImageStack>
{
  public:
    FilterFactory* clone() const OVERRIDE { return new FilterFactory(*this); }

    void attach_ui( simparm::NodeHandle at,
                    std::function<void()> traits_change_callback) OVERRIDE { 
        listening[0] = config.first_frame.value.notify_on_value_change(
                traits_change_callback);
        listening[1] = config.last_frame.value.notify_on_value_change( 
                traits_change_callback);
        config.attach_ui( at ); 
    }

    std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) OVERRIDE {
        if ( config.first_frame() > 0 * camera::frame 
            || config.last_frame().is_initialized() ) {
            return make_unique<Source>( std::move(input), config.first_frame(), config.last_frame() );
        } else {
            return std::move(input);
        }
    }

    boost::shared_ptr<const input::Traits<engine::ImageStack>> make_meta_info(
        boost::shared_ptr<const input::Traits<engine::ImageStack>> input_meta_info) OVERRIDE {
        auto mine = boost::make_shared<input::Traits<engine::ImageStack>>(*input_meta_info);
        mine->image_number().range().first = config.first_frame();
        if ( config.last_frame().is_initialized() )
            mine->image_number().range().second = config.last_frame();
        return mine;
    }

  private:
    Config config;
    simparm::BaseAttribute::ConnectionStore listening[2];
};

Config::Config() 
: name_object("ROIFilter", "Image selection filter"),
  first_frame("FirstImage", 0 * camera::frame),
  last_frame( "LastImage", boost::optional< frame_index >() ) {
    first_frame.set_user_level( simparm::Intermediate );
    last_frame.set_user_level( simparm::Intermediate );
}

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create() {
    return make_unique<FilterFactory>();
}

}
}

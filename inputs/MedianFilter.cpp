#include "MedianFilter.h"

#include <queue>

#include <simparm/BoostUnits.h>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

#include "image/extend.h"
#include "image/Image.h"
#include "image/slice.h"
#include "input/AdapterSource.h"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "input/Method.hpp"
#include "localization/Traits.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"
#include "UnitEntries/FrameEntry.h"
#include "units/frame_count.h"

namespace dStorm {
namespace ROIFilter {

struct Config {
    simparm::Object name_object;
    simparm::BoolEntry apply_filter;
    IntFrameEntry width, stride;

    Config();
    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = name_object.attach_ui(at);
        apply_filter.attach_ui( r );
        width.attach_ui( r ); 
        stride.attach_ui( r ); 
    }
};

class Source
: public input::AdapterSource<engine::ImageStack>
{
    typedef engine::StormPixel StormPixel;

    const int half_width, width, stride;
    int remaining_stride;
    bool initial_median_computed, upstream_is_empty;

    std::queue<engine::ImageStack> outgoing;
    std::vector<dStorm::Image<StormPixel, 3>> median_values;

    void attach_local_ui_( simparm::NodeHandle ) {}

    template <class UpdateFunction>
    void ComputeMedian(const UpdateFunction& update) {
        for (size_t plane_index = 0; plane_index < median_values.size(); ++plane_index) {
            engine::Image2D starts = median_values[plane_index].slice(0, 0 * camera::pixel);
            for (auto i = starts.begin(); i != starts.end(); ++i) {
                update(&*i, (&*i) + width, plane_index, i.position());
            }
        }
    }

    void UpdateMedian(const engine::ImageStack& incoming, const engine::ImageStack& outgoing) {
        ComputeMedian([&](StormPixel* begin, StormPixel* end, int plane, const engine::Image2D::Position& position) {
            StormPixel new_value = incoming.plane(plane)(position);
            StormPixel old_value = outgoing.plane(plane)(position);
            StormPixel* insertion_point = std::lower_bound(begin, end, new_value);
            StormPixel* deletion_point = std::find(begin, end, old_value);

            assert(deletion_point < end);
            if (insertion_point < deletion_point) {
                std::move_backward(insertion_point, deletion_point, deletion_point + 1);
            } else if (insertion_point == end) {
                std::move(deletion_point + 1, end, deletion_point);
                insertion_point = end - 1;
            } else {
                std::move(deletion_point + 1, insertion_point + 1, deletion_point);
            }
            *insertion_point = new_value;
        });
    }

    bool GetKeyImage(engine::ImageStack& input) {
	for (int j = 0; j < stride; ++j) {
	    if (!base().GetNext(0, &input)) {
		upstream_is_empty = true;
		return false;
	    }

	    outgoing.push(input);
	}
	return true;
    }

    bool ComputeInitialMedian() {
	engine::ImageStack input;
	if (!GetKeyImage(input)) { return false; }
	std::vector<engine::ImageStack> inputs(half_width + 2, input);

	for (int i = 0; i < half_width - 1; ++i) {
	    if (!GetKeyImage(input)) { return false; }
	    inputs.push_back(input);
	}

	for (int i = 0; i < width; ++i) {
	    for (int plane = 0; plane < inputs[i].plane_count(); ++plane) {
		engine::Image2D image = inputs[i].plane(plane);
		std::copy(image.begin(), image.end(),
			  median_values[plane].slice(0, i * camera::pixel).begin());
	    }
	}

	remaining_stride = stride;
	return true;
    }

    bool GetNext(int thread, engine::ImageStack* output) OVERRIDE {
	assert(thread == 0);

	if (!initial_median_computed) {
	    if (!ComputeInitialMedian()) {
		throw std::runtime_error("Too few images in input to compute "
		    "background median. At least width * stride images are "
		    "needed.");
	    }
	    initial_median_computed = true;
	}

	engine::ImageStack image;
	if (!upstream_is_empty && base().GetNext(0, &image)) {
	    outgoing.push(image);
	    if (--remaining_stride == 0) {
		UpdateMedian(image, outgoing.front());
		remaining_stride = stride;
	    }
	}

	if (outgoing.empty()) {
	    return false;
	} else {
	    *output = outgoing.front();
	    for (const auto& median : median_values) {
		image.push_back_background(median.slice(0, half_width * camera::pixel));
	    }
	    outgoing.pop();
	    return true;
	}
    }

  public:
    Source( std::auto_ptr< input::Source<engine::ImageStack> > upstream,
            frame_index width, frame_index stride)
        : input::AdapterSource<engine::ImageStack>(upstream),
          half_width(width.value() / 2), width(width.value()), stride(stride.value()) {
	if (width.value() % 2 == 0) {
	    throw std::runtime_error("Median width must be an odd number");
	}
    }
    Source* clone() const { throw std::logic_error("clone() for MedianFilter::Source not implemented"); }

    void modify_traits( input::Traits<engine::ImageStack>& p)
    {
        for (int i = 0; i < p.plane_count(); ++i) {
            dStorm::ImageTypes<3>::Size size;
            size[0] = width * camera::pixel;
            size[1] = p.plane(i).image.size[0];
            size[2] = p.plane(i).image.size[1];
            median_values.emplace_back(size);
        }
    }
};

class ChainLink 
: public input::Method<ChainLink>
{
    friend class input::Method<ChainLink>;

    Config config;

    template <typename Type>
    void update_traits( input::MetaInfo&, input::Traits<Type>& traits ) {}
    template <typename Type>
    void notice_traits( const input::MetaInfo&, const input::Traits<Type>& ) {}

    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) {
        return p.release();
    }

    input::Source<engine::ImageStack>* make_source( std::auto_ptr< input::Source<engine::ImageStack> > p ) {
        return new Source( p, config.width(), config.stride() );
    }

  public:
    void attach_ui( simparm::NodeHandle at ) { 
        config.attach_ui( at ); 
    }
    static std::string getName() {
        return "TemporalMedianFilter";
    }
};

Config::Config() 
: name_object(ChainLink::getName(), "Temporal median filter"),
  apply_filter("ApplyFilter", false),
  width("Number of images in median", 21 * camera::frame),
  stride( "Stride between key images", 10 * camera::frame )
{
    apply_filter.set_user_level( simparm::Intermediate ),
    width.set_user_level( simparm::Expert );
    stride.set_user_level( simparm::Expert );
}

std::auto_ptr<input::Link> make_link() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

std::auto_ptr<input::Source<engine::ImageStack>> make_source(
        std::auto_ptr<input::Source<engine::ImageStack>> upstream,
        frame_index width, frame_index stride) {
    return std::auto_ptr<input::Source<engine::ImageStack>>(
        new Source(upstream, width, stride));
}

}
}

#include "MedianFilter.h"

#include <queue>

#include "simparm/BoostUnits.h"
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

#include "image/extend.h"
#include "image/Image.h"
#include "image/slice.h"
#include "input/AdapterSource.h"
#include "input/FilterFactory.h"
#include "input/FilterFactoryLink.h"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "localization/Traits.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"
#include "simparm/ManagedChoiceEntry.h"
#include "simparm/ObjectChoice.h"
#include "UnitEntries/FrameEntry.h"
#include "units/frame_count.h"

namespace dStorm {
namespace median_filter {

struct Config {
    simparm::Object name_object;
    IntFrameEntry width, stride;

    Config();
    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = name_object.attach_ui(at);
        width.attach_ui( r ); 
        stride.attach_ui( r ); 
    }
};

class Source
: public input::AdapterSource<engine::ImageStack>
{
    typedef engine::StormPixel StormPixel;

    const int half_width, width, stride;
    int overbuffered_images, time_to_median_update;
    bool initial_median_computed, upstream_is_empty;

    std::queue<engine::ImageStack> output_buffer;
    std::queue<engine::ImageStack> median_buffer;
    std::vector<dStorm::Image<StormPixel, 3>> median_values;

    void attach_local_ui_( simparm::NodeHandle ) {}

    void replace_in_sorted_range(StormPixel* begin, StormPixel* end,
                                 StormPixel old_value, StormPixel new_value) {
        StormPixel* insertion_point = std::lower_bound(begin, end, new_value);
        StormPixel* deletion_point = std::lower_bound(begin, end, old_value);

        assert(deletion_point < end);
        assert(*deletion_point == old_value);
        assert(std::is_sorted(begin, end));
        if (insertion_point <= deletion_point) {
            std::move_backward(insertion_point, deletion_point, deletion_point + 1);
            *insertion_point = new_value;
        } else {
            std::move(deletion_point + 1, insertion_point, deletion_point);
            *(insertion_point-1) = new_value;
        }
        assert(std::is_sorted(begin, end));
    }

    void UpdateMedian(const engine::ImageStack& incoming, const engine::ImageStack& outgoing) {
        for (size_t plane_index = 0; plane_index < median_values.size(); ++plane_index) {
            engine::Image2D starts = median_values[plane_index].slice(0, 0 * camera::pixel);
            for (auto i = starts.begin(); i != starts.end(); ++i) {
                replace_in_sorted_range(&*i, &*i + width,
                    outgoing.plane(plane_index)(i.position()),
                    incoming.plane(plane_index)(i.position()));
            }
        }
    }

    bool ComputeInitialMedian() {
        for (int i = 0; i < width * stride; ++i) {
            engine::ImageStack input;
            if (!base().GetNext(0, &input)) {
                return false;
            }

            output_buffer.push(input);
            if (i % stride == 0) {
                median_buffer.push(input);
                for (int plane = 0; plane < input.plane_count(); ++plane) {
                    engine::Image2D image = input.plane(plane);
                    std::copy(image.begin(), image.end(),
                              median_values[plane].slice(0, (i/stride) * camera::pixel).begin());
                }
            }
        }

        for (const auto& image : median_values) {
            engine::Image2D starts = image.slice(0, 0 * camera::pixel);
            for (auto& i : starts) {
                std::sort(&i, &i + width);
            }
        }

        overbuffered_images = (half_width + 1) * stride;
        time_to_median_update = 0;
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

        if (overbuffered_images > 0) {
            --overbuffered_images;
        } else {
            engine::ImageStack image;
            if (!upstream_is_empty && base().GetNext(0, &image)) {
                output_buffer.push(image);
                if (time_to_median_update > 0) {
                    --time_to_median_update;
                } else {
                    UpdateMedian(image, median_buffer.front());
                    time_to_median_update = stride - 1;
                    median_buffer.pop();
                    median_buffer.push(image);
                }
            } else {
                upstream_is_empty = true;
            }
        }

	if (output_buffer.empty()) {
	    return false;
	} else {
	    *output = output_buffer.front();
	    for (int plane = 0; plane < output->plane_count(); ++plane) {
		output->set_background(plane,
                    median_values[plane].slice(0, half_width * camera::pixel).deep_copy());
	    }
	    output_buffer.pop();
	    return true;
	}
    }

  public:
    Source( std::unique_ptr< input::Source<engine::ImageStack> > upstream,
            frame_index width, frame_index stride)
        : input::AdapterSource<engine::ImageStack>(std::move(upstream)),
          half_width(width.value() / 2), width(width.value()), stride(stride.value()),
          initial_median_computed(false), upstream_is_empty(false) {
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

            p.plane(i).has_background_estimate = true;
        }
    }

    void dispatch(Messages m) {
        base().dispatch(m);
        if (m.test(RepeatInput)) {
            initial_median_computed = false;
            upstream_is_empty = false;
	    while (!output_buffer.empty()) {
                output_buffer.pop();
            }
	    while (!median_buffer.empty()) {
                median_buffer.pop();
            }
        }
    }
};

class BackgroundFilterChoice : public simparm::ObjectChoice {
  public:
    BackgroundFilterChoice(std::string name, std::string desc)
        : simparm::ObjectChoice(name, desc) {}
    virtual BackgroundFilterChoice* clone() const = 0;
    virtual std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) = 0;
};

class MedianFilterChoice : public BackgroundFilterChoice {
  public:
    MedianFilterChoice()
        : BackgroundFilterChoice("TemporalMedianFilter", "Temporal median filter") {}
    MedianFilterChoice* clone() const { return new MedianFilterChoice(*this); }
    void attach_ui(simparm::NodeHandle at) { config.attach_ui(attach_parent(at)); }
    std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) {
        return std::unique_ptr<input::Source<engine::ImageStack>>(
            new Source(std::move(input), config.width(), config.stride()));
    }

  private:
    Config config;
};

class NoFilterChoice : public BackgroundFilterChoice {
  public:
    NoFilterChoice() : BackgroundFilterChoice("None", "No background filter") {}
    NoFilterChoice* clone() const { return new NoFilterChoice(*this); }
    void attach_ui(simparm::NodeHandle at) { attach_parent(at); }
    std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) {
        return std::move(input);
    }
};

class BackgroundFilterLink : public input::FilterFactory<engine::ImageStack> {
  public:
    BackgroundFilterLink() : backend("BackgroundFilter") {
        backend.addChoice(new NoFilterChoice());
        backend.addChoice(new MedianFilterChoice());
        backend.set_user_level( simparm::Intermediate );
    }

    BackgroundFilterLink* clone() const { return new BackgroundFilterLink(*this); }
    std::string getName() const { return "BackgroundFilter"; }
    void attach_ui(simparm::NodeHandle at, std::function<void()>) { backend.attach_ui(at); }
    std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) {
        return backend.active_choice().make_source(std::move(input));
    }
    boost::shared_ptr<const input::Traits<engine::ImageStack>> make_meta_info(
        input::MetaInfo& meta_info,
        boost::shared_ptr<const input::Traits<engine::ImageStack>> input_meta_info) {
        return input_meta_info;
    }
  private:
    simparm::ManagedChoiceEntry<BackgroundFilterChoice> backend;
};

Config::Config() 
: name_object("MedianFilter", "Temporal median filter"),
  width("MedianWidth", "Number of images in median", 11 * camera::frame),
  stride("MedianStride", "Stride between key images", 5 * camera::frame )
{
    width.set_user_level( simparm::Intermediate );
    stride.set_user_level( simparm::Intermediate );
}

std::auto_ptr<input::Link> make_link() {
    return CreateLink(std::unique_ptr<BackgroundFilterLink>(new BackgroundFilterLink()));
}

std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> upstream,
        frame_index width, frame_index stride) {
    return std::auto_ptr<input::Source<engine::ImageStack>>(
        new Source(std::move(upstream), width, stride));
}

}
}

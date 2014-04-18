#include "inputs/MedianFilter_test.h"

#include <vector>

#include <boost/test/unit_test.hpp>

#include "engine/InputTraits.h"
#include "inputs/MedianFilter.h"

namespace dStorm {
namespace median_filter {

class FakeImageSource : public input::Source<engine::ImageStack> {
  public:
    FakeImageSource(const std::vector<engine::ImageStack>& images) : images(images), current(images.begin()) {}

  private:
    bool GetNext(int thread, engine::ImageStack* output) OVERRIDE {
	if (current != images.end()) {
	    *output = *current;
	    ++current;
	    return true;
	} else {
	    return false;
	}
    }

    TraitsPtr get_traits(BaseSource::Wishes wishes) OVERRIDE {
	return TraitsPtr(new Traits());
    }

    void attach_ui_( simparm::NodeHandle ) OVERRIDE {
    }

    void set_thread_count(int num_threads) { assert(num_threads == 1); }
    void dispatch(Messages m) { assert(!m.any()); }
    Capabilities capabilities() const { return Capabilities(); }

    std::vector<engine::ImageStack> images;
    std::vector<engine::ImageStack>::const_iterator current;
};

void TestRandomSequence() {
    int frames = 100;
    int stride = 3;
    int planes = 4;
    int image_width = 30;
    int image_height = 50;
    int window_width = 11;

    srand(0);
    std::vector<engine::ImageStack> images;
    for (int image = 0; image < frames; ++image) {
	engine::ImageStack three_d;
	for (int plane = 0; plane < planes; ++plane) {
	    engine::Image2D two_d(engine::Image2D::Size(image_width * boost::units::camera::pixel,
                                                        image_height * boost::units::camera::pixel));
	    for (int x = 0; x < image_width; ++x) {
		for (int y = 0; y < image_height; ++y) {
		    two_d(x, y) = rand();
		}
	    }
	    three_d.push_back(two_d);
	}
	images.push_back(three_d);
    }

    std::auto_ptr<input::Source<engine::ImageStack>> filter(
        make_source(
            std::auto_ptr<input::Source<engine::ImageStack>>(
                new FakeImageSource(images)),
            frame_index::from_value(window_width),
            frame_index::from_value(stride)));

    engine::ImageStack output;
    for (int i = 0; i < frames; ++i) {
        BOOST_CHECK(filter->GetNext(0, &output));

        for (int p = 0; p < planes; ++p) {
	    for (int x = 0; x < image_width; ++x) {
		for (int y = 0; y < image_height; ++y) {
                    std::vector<int> window_values(window_width, -1);
                    for (int k = 0; k < window_width; ++k) {
                        int key = std::max(0, std::min(i / stride + k, frames / stride));
                        window_values[k] = images[key * stride].plane(p)(x, y);
                    }
                    std::sort(window_values.begin(), window_values.end());
                    int naive_median = window_values[window_width/2];
                    BOOST_CHECK_EQUAL(naive_median, output.plane(p)(x,y));
                }
            }
        }
    }

    BOOST_CHECK(!filter->GetNext(0, &output));
}

void unit_test() {
    TestRandomSequence();
}

}
}

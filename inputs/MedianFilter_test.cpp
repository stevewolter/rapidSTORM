#include "inputs/MedianFilter.h"
#include "dejagnu.h"

namespace dStorm {
namespace median_filter {

class FakeImageSource : public input::Source<engine::ImageStack> {
  public:
    FakeImageSource(const vector<engine::ImageStack>& images) : images(images), current(images.begin()) {}

  private:
    bool GetNext(int thread, Type* output) OVERRIDE {
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

    void set_thread_count(int num_threads) { assert(num_threads == 1); }
    void dispatch(Messages m) { assert(!m.set()); }
    Capabilities capabilities() const { return Capabilities(); }

    vector<engine::ImageStack> images;
    vector<engine::ImageStack>::const_iterator current;
};

void TestLongSequence(TestState& state) {
    vector<engine::ImageStack> images;
    for (int image = 0; image < 100; ++image) {
	engine::ImageStack three_d;
	for (int plane = 0; plane < 3; ++plane) {
	    engine::Image2D two_d(30, 50);
	    for (int x = 0; x < 30; ++x) {
		for (int y = 0; y < 30; ++y) {
		    i(x, y) = x * 3 + y * 5 + image * 8 + plane;
		}
	    }
	    three_d.push_back(two_d);
	}
	images.push_back(three_d);
    }

    std::auto_ptr<input::Link> link = make_link();
    link->make_source();
}

void unit_test( TestState& state ) {
    TestLongSequence(state);
}

}
}

#include "inputs/MedianFilter_test.h"

#include <vector>

#include <boost/test/unit_test.hpp>

#include "engine/InputTraits.h"
#include "input/FakeSource.h"
#include "inputs/MedianFilter.h"

namespace dStorm {
namespace median_filter {

void TestRandomSequence(int window_width, int stride, int frames) {
    int planes = 4;
    int image_width = 30;
    int image_height = 50;

    srand(0);

    input::Traits<engine::ImageStack> meta_info;
    for (int plane = 0; plane < planes; ++plane) {
        image::MetaInfo<2> plane_meta_info;
        plane_meta_info.size = engine::Image2D::Size(
            image_width * boost::units::camera::pixel,
            image_height * boost::units::camera::pixel);
        meta_info.push_back(plane_meta_info, traits::Optics());
    }

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
                new input::FakeSource<engine::ImageStack>(meta_info, images)),
            frame_index::from_value(window_width),
            frame_index::from_value(stride)));
    filter->get_traits();

    bool all_equal = true;
    bool did_reset = false;
    engine::ImageStack output;
    for (int i = 0; i < frames; ++i) {
        BOOST_CHECK(filter->GetNext(0, &output));

        if (i == frames / 2 && !did_reset) {
            i = -1;
            did_reset = true;
            input::BaseSource::Messages m;
            m.set(input::BaseSource::RepeatInput);
            filter->dispatch(m);
            continue;
        }

        for (int p = 0; p < planes; ++p) {
	    for (int x = 0; x < image_width; ++x) {
		for (int y = 0; y < image_height; ++y) {
                    std::vector<int> window_values(window_width, -1);
                    int natural_window_start = i / stride - window_width / 2;
                    int max_window_start = (frames - 1) / stride - window_width + 1;
                    int window_start = std::max(0,
                            std::min(natural_window_start, max_window_start));
                    for (int k = 0; k < window_width; ++k) {
                        window_values[k] = images[(k + window_start) * stride].plane(p)(x, y);
                    }
                    std::sort(window_values.begin(), window_values.end());
                    int naive_median = window_values[window_width/2];
                    if (naive_median != output.background(p)(x,y)) {
                        all_equal = false;
                    }
                }
            }
        }
    }
    BOOST_CHECK(all_equal);

    BOOST_CHECK(!filter->GetNext(0, &output));
}

void TestNoOp() {
    TestRandomSequence(1, 1, 100);
}

void TestSimpleMedian() {
    TestRandomSequence(3, 1, 100);
}

void TestNoOpWithStride() {
    TestRandomSequence(1, 2, 100);
}

void TestMedianWithStride() {
    TestRandomSequence(11, 3, 100);
}

void TestEvenNumberOfImages() {
    TestRandomSequence(11, 3, 99);
}

void TestExactSizeOfSequence() {
    TestRandomSequence(11, 3, 33);
}

void TestTooSmallSequence() {
    try {
        TestRandomSequence(11, 3, 10);
        BOOST_CHECK(false);
    } catch (const std::runtime_error& e) {
    }
}

boost::unit_test::test_suite* unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "median_filter" );
    rv->add( BOOST_TEST_CASE( &TestNoOp ) );
    rv->add( BOOST_TEST_CASE( &TestSimpleMedian ) );
    rv->add( BOOST_TEST_CASE( &TestNoOpWithStride ) );
    rv->add( BOOST_TEST_CASE( &TestMedianWithStride ) );
    rv->add( BOOST_TEST_CASE( &TestEvenNumberOfImages ) );
    rv->add( BOOST_TEST_CASE( &TestExactSizeOfSequence ) );
    rv->add( BOOST_TEST_CASE( &TestTooSmallSequence ) );
    return rv;
}

}
}

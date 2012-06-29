namespace boost { namespace unit_test { class test_suite; } }
namespace dStorm {
template <typename PixelType, int Dimensions> class Image;
namespace image {

template <int Dimensions> struct MetaInfo;
boost::unit_test::test_suite* unit_test_suite();

}
}

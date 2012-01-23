#ifndef DSTORM_TEST_XENOPHONS_STORY_H
#define DSTORM_TEST_XENOPHONS_STORY_H

#include <vector>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/output/LocalizedImage_traits.h>

namespace dStorm {
namespace test {

input::Traits< output::LocalizedImage > xenophon_traits();
std::vector< output::LocalizedImage > xenophon();

}
}

#endif

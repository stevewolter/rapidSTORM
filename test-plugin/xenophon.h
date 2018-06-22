#ifndef DSTORM_TEST_XENOPHONS_STORY_H
#define DSTORM_TEST_XENOPHONS_STORY_H

#include <vector>
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace test {

input::Traits< output::LocalizedImage > xenophon_traits();
std::vector< output::LocalizedImage > xenophon();

}
}

#endif

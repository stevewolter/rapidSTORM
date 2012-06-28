#ifndef DSTORM_VIEWER_FWD_H
#define DSTORM_VIEWER_FWD_H

#include <memory>

namespace dStorm {
namespace output { class OutputSource; }
namespace viewer {

std::auto_ptr<output::OutputSource> make_output_source();
std::auto_ptr<output::OutputSource> make_density_map_output_source();

}
}

#endif

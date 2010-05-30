#ifndef DSTORM_VIEWER_BACKEND_H
#define DSTORM_VIEWER_BACKEND_H

#include <dStorm/output/Output.h>

namespace dStorm {
namespace viewer {

struct Backend
{
    virtual ~Backend() {}
    virtual output::Output& getForwardOutput() = 0;

    virtual void save_image(std::string filename, bool with_key) = 0;

    virtual void set_histogram_power(float power) = 0;
    virtual void set_resolution_enhancement(float re) = 0;

};

}
}

#endif

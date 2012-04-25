#ifndef DSTORM_FORM_FITTER_INPUT_H
#define DSTORM_FORM_FITTER_INPUT_H

#include "Config.h"
#include <dStorm/output/Output.h>
#include "guf/Spot.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace estimate_psf_form {

struct Input {
    boost::shared_ptr< const input::Traits< engine::ImageStack > > traits;
    guf::Spot width;
    const int number_of_spots;
    int fluorophore_count;

    Input( const Config&, const output::Output::Announcement&, guf::Spot width );
};

}
}

#endif

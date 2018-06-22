#ifndef DSTORM_FORM_FITTER_INPUT_H
#define DSTORM_FORM_FITTER_INPUT_H

#include "estimate_psf_form/Config.h"
#include "output/Output.h"
#include "types/samplepos.h"
#include "guf/Spot.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace estimate_psf_form {

struct Input {
    boost::shared_ptr< const input::Traits< engine::ImageStack > > traits;
    samplepos::Scalar width;
    const int number_of_spots;
    int fluorophore_count;

    Input( const Config&, const output::Output::Announcement&, samplepos::Scalar width );
};

}
}

#endif

#ifndef DSTORM_FORM_FITTER_INPUT_H
#define DSTORM_FORM_FITTER_INPUT_H

#include "Config.h"
#include <dStorm/output/Output.h>
#include "dStorm/types/samplepos.h"
#include "guf/Spot.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace estimate_psf_form {

struct Input {
    typedef Eigen::Matrix<samplepos::Scalar,2,1,Eigen::DontAlign> Width;
    boost::shared_ptr< const input::Traits< engine::ImageStack > > traits;
    Width width;
    const int number_of_spots;
    int fluorophore_count;

    Input( const Config&, const output::Output::Announcement&, Width width );
};

}
}

#endif

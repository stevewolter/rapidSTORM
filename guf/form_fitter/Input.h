#ifndef DSTORM_FORM_FITTER_INPUT_H
#define DSTORM_FORM_FITTER_INPUT_H

#include "Config.h"
#include <dStorm/output/Output.h>
#include "guf/guf/TransformedImage.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace form_fitter {

struct Input {
    typedef guf::TransformedImage< si::length > Transformed;

    boost::shared_ptr< const input::Traits< engine::Image > > traits;
    boost::ptr_vector<Transformed> transforms;
    const int number_of_spots;
    int fluorophore_count;

    Input( const Config&, const output::Output::Announcement& );
};

}
}

#endif

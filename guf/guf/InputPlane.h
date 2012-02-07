#ifndef DSTORM_GUF_INPUTPLANE_H
#define DSTORM_GUF_INPUTPLANE_H

#include <Eigen/StdVector>
#include "TransformedImage.h"
#include "Spot.h"
#include "Config.h"
#include <boost/optional/optional.hpp>
#include "guf/psf/LengthUnit.h"
#include <dStorm/engine/JobInfo.h>
#include <dStorm/traits/Projection.h>

namespace dStorm {
namespace guf {

template <int Dim> class Statistics;
class DataPlane;
class InputPlane {
    typedef guf::TransformedImage< PSF::LengthUnit > TransformedImage;

    traits::Projection::ImagePosition im_size;
    TransformedImage transformation;

    quantity< camera::intensity > photon_response_;
    quantity< camera::intensity, int > dark_current;
    boost::optional< float > background_noise_variance_;
    bool can_do_disjoint_fitting, has_precision,
         do_mle, use_floats;

    static guf::Spot
        make_max_distance( const dStorm::engine::JobInfo& info, int plane_number );

    friend class DataPlane;
    class instantiate_appropriate_data;

  public:
    typedef dStorm::engine::Image2D Image;
    InputPlane( const Config&, const dStorm::engine::JobInfo&, int plane_index );
    std::auto_ptr< DataPlane >
        set_image( const Image& image, const guf::Spot& position ) const;
    int get_fit_window_width(const guf::Spot& at) const;
    template <typename Data>
    inline const Statistics<2> set_data(Data&, const Image&, const Spot&) const;
    bool can_compute_localization_precision() const { return has_precision; }
    float background_noise_variance() const { return *background_noise_variance_; }
    quantity< camera::intensity > photon_response() const { return photon_response_; }
};

}
}

#endif

#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H

#include "debug.h"

#include "binning/binning.h"
#include <boost/ptr_container/ptr_array.hpp>
#include <dStorm/helpers/clone_ptr.hpp>

namespace dStorm {
namespace density_map {

template <int Dim>
class Coordinates
{
public:
    struct ResultRow {
        Eigen::Matrix<float, Dim, 1> position, position_uncertainty;
        float intensity, intensity_uncertainty;
    };
    typedef std::vector<ResultRow> Result;
private:
    typedef binning::Scaled ScaledAxis;
    typedef binning::Unscaled UnscaledAxis;

    boost::ptr_array<ScaledAxis, Dim> xy;
    boost::clone_ptr<UnscaledAxis> intensity;

public:
    Coordinates( boost::ptr_array<ScaledAxis, Dim> dims, std::auto_ptr<UnscaledAxis> intensity)
        : xy(dims), intensity(intensity) {}
    Coordinates* clone() const { return new Coordinates(*this); }
    ~Coordinates() {}

    void announce(const output::Output::Announcement& a) { 
        for (int i = 0; i < Dim; ++i) xy[i].announce(a);
        intensity->announce(a);
    }
    Eigen::Matrix<quantity<camera::length>, Dim, 1> get_size() {
        Eigen::Matrix<quantity<camera::length>, Dim, 1> rv;
        for (int i = 0; i < Dim; ++i)
            rv[i] = xy[i].get_size() * camera::pixel;
        return rv;
    }

    typename image::MetaInfo<Dim>::Resolutions get_resolution() {
        typename image::MetaInfo<Dim>::Resolutions rv;
        for (int i = 0; i < 2; ++i) {
            rv[i] = xy[i].resolution();
        }
        return rv;
    }
    int bin_points( const output::LocalizedImage& l, Result& r ) {
        int rv = 0;
        for ( output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i ) {
            bool is_good = true;
            for (int d = 0; d < Dim; ++d)
                is_good = is_good && bin( *i, xy[d], r[rv].position[ d ], r[rv].position_uncertainty[ d ] );
            is_good = is_good && bin( *i, *intensity, r[rv].intensity, r[rv].intensity_uncertainty );
            if ( is_good ) ++rv;
        }
        return rv;
    }

  private:
    bool bin( const Localization& l, const UnscaledAxis& b, float& target, float& uncertainty ) {
        boost::optional<float> f = b.bin_point(l), u = b.get_uncertainty( l );
        if ( f.is_initialized() ) target = *f;
        if ( u.is_initialized() ) uncertainty = *u; else uncertainty = 0;
        return f.is_initialized();
    }
};

}
}

#endif

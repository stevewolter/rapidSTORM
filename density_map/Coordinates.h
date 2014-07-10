#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H

#include "debug.h"

#include "binning/binning.h"
#include <boost/ptr_container/ptr_array.hpp>
#include "helpers/clone_ptr.hpp"

namespace dStorm {
namespace density_map {

template <int Dim>
class Coordinates
{
public:
    struct ResultRow {
        Eigen::Matrix<float, Dim, 1> position, position_uncertainty;
        float intensity;
    };
    typedef std::vector<ResultRow> Result;
private:
    typedef binning::Scaled ScaledAxis;
    typedef binning::Unscaled UnscaledAxis;

    boost::ptr_array<ScaledAxis, Dim> xy;
    boost::ptr_array<UnscaledAxis, Dim> xy_uncertainties;
    clone_ptr<UnscaledAxis> intensity;

public:
    Coordinates( boost::ptr_array<ScaledAxis, Dim> dims,
                 boost::ptr_array<ScaledAxis, Dim> spatial_uncertainties,
                 std::unique_ptr<UnscaledAxis> intensity)
        : xy(dims), xy_uncertainties(spatial_uncertainties), intensity(intensity.release()) {}
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
            if (bin_point(*i, r[rv])) {
                ++rv;
            }
        }
        return rv;
    }

  private:
    bool bin_point( const Localization& l, ResultRow& r ) {
        for (int d = 0; d < Dim; ++d) {
            boost::optional<float> value = xy[d].bin_point(l);
            if (!value) return false;
            r.position[d] = *value;
            r.position_uncertainty[d] = xy_uncertainties[d].bin_point(l).get_value_or(0);
        }
        boost::optional<float> intensity = this->intensity->bin_point(l);
        if (!intensity) return false;
        r.intensity = *intensity;
        return true;
    }
};

}
}

#endif

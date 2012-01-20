#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H

#include "debug.h"

#include "BinnedLocalizations.h"
#include "../output/binning/binning.h"
#include "../traits/scalar.h"
#include <boost/ptr_container/ptr_array.hpp>
#include "../helpers/clone_ptr.hpp"

namespace dStorm {
namespace outputs {

namespace binning_strategy {

template <int Dim>
struct ComponentWise
: public BinningStrategy<Dim>
{
  private:
    typedef output::binning::Scaled ScaledBin;
    typedef output::binning::Unscaled UnscaledBin;

    boost::ptr_array<ScaledBin, Dim> xy;
    boost::clone_ptr<UnscaledBin> intensity;

  public:
    ComponentWise( boost::ptr_array<ScaledBin, Dim> dims, std::auto_ptr<UnscaledBin> intensity)
        : xy(dims), intensity(intensity) {}
    ComponentWise* clone() const { return new ComponentWise(*this); }
    ~ComponentWise() {}

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

    traits::Optics<2>::Resolutions get_resolution() {
        traits::Optics<2>::Resolutions rv;
        for (int i = 0; i < 2; ++i) {
            rv[i] = xy[i].resolution();
        }
        return rv;
    }
    int bin_points( const output::LocalizedImage& l, typename BinningStrategy<Dim>::Result& r ) {
        int rv = 0;
        for ( output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i ) {
            bool is_good = true;
            for (int d = 0; d < Dim; ++d)
                is_good = is_good && bin( *i, xy[d], r( rv, d ) );
            bin( *i, *intensity, r(rv,Dim) );
            if ( is_good ) ++rv;
        }
        return rv;
    }

  private:
    bool bin( const Localization& l, const UnscaledBin& b, float& target ) {
        boost::optional<float> f = b.bin_point(l);
        if ( f.is_initialized() )
            target = *f;
        return f.is_initialized();
    }
};

}

}
}

#endif

#include "fit_window/FitWindowCutter.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>

#include "engine/Image.h"
#include "fit_window/fit_position_out_of_range.h"

namespace dStorm {
namespace fit_window {

using namespace boost::accumulators;

FitWindowCutter::FitWindowCutter(const Config& c, const dStorm::engine::InputTraits& traits,
            std::set<int> desired_fit_window_widths, int fit_window_width_slack)
    : desired_fit_window_widths(desired_fit_window_widths), fit_window_width_slack(fit_window_width_slack) {
    auto window_size = Spot::Constant( quantity<si::length>( c.fit_window_size() ).value() * 1E6 );
    for (int i = 0; i < traits.plane_count(); ++i) {
        optics.emplace_back( new Optics(window_size, traits.plane(i)) );
    }
}

Plane FitWindowCutter::cut_region_of_interest(
    const Optics& optics,
    const dStorm::engine::Image2D& image,
    const Spot& position
) {
    const float background_part = 0.25;

    Plane result;
    result.optics = &optics;

    result.pixel_size = quantity<si::area>(optics.pixel_size(position)).value() * 1E12;

    /* Initialize iteratively computed statistics */
    for (int d = 0; d < 2; ++d) {
        result.min_coordinate[d] = std::numeric_limits<double>::max();
        result.max_coordinate[d] = std::numeric_limits<double>::min();
    }
    result.integral = 0;
    result.highest_pixel_index = 0;
    result.window_width = -1;
    result.peak_intensity = 0;

    Optics::OptPixel window_width;
    if (optics.supports_guaranteed_row_width()) {
        int natural_width = optics.get_fit_window_width(position);
        for (int i = natural_width - fit_window_width_slack; i < natural_width + 2 + fit_window_width_slack; ++i) {
            if (desired_fit_window_widths.count(i) > 0) {
                result.window_width = i;
                window_width = i * camera::pixel;
            }
        }
    }

    traits::Projection::ROI points 
        = optics.get_region_of_interest( position, window_width );

    if ( points.size() <= 0 ) 
        throw fit_position_out_of_range();

    result.points.clear();
    result.points.reserve( points.size() );
    /* Keep a list of the inserted pixel values for later statistical usage */
    std::vector<float> pixels;
    pixels.reserve( points.size() );
    for ( traits::Projection::ROI::const_iterator i = points.begin(); i != points.end(); ++i )
    {
        assert( image.contains( i->image_position ) );

        DataPoint data_point;
        for (int d = 0; d < 2; ++d) {
            double pos = quantity<si::length>(i->sample_position[d]).value() * 1E6;
            data_point.position[d] = pos;
            result.min_coordinate[d] = std::min( pos, result.min_coordinate[d] );
            result.max_coordinate[d] = std::max( pos, result.max_coordinate[d] );
        }

        const double value = 
            std::max( 0.0, optics.absolute_in_photons( image( i->image_position ) * camera::ad_count ) );
        data_point.value = value;
        pixels.push_back(value);
        result.integral += value;
        if ( value >= result.peak_intensity ) {
            result.highest_pixel_index = points.size();
        }

        result.points.push_back(data_point);
    }

    assert( ! pixels.empty() );

    std::vector<float>::iterator qp = pixels.begin() + pixels.size() * background_part;
    std::nth_element( pixels.begin(), qp, pixels.end());
    result.background_estimate = *qp;

    accumulator_set<double, stats<tag::weighted_variance(lazy)>, double> acc[2];

    for (const DataPoint& point : result.points) {
        for (int dim = 0; dim < 2; ++dim) {
            double offset = point.position[dim] - result.points[result.highest_pixel_index].position[dim];
            double intensity_above_background = 
                std::max(0.0, double(point.value - result.background_estimate));
            acc[dim]( offset, weight = intensity_above_background );
        }
    }
    for (int dim = 0; dim < 2; ++dim)
        result.standard_deviation[dim] = sqrt( weighted_variance(acc[dim]) );

    return result;
}

std::vector<Plane> FitWindowCutter::cut_region_of_interest(
    const dStorm::engine::ImageStack& image,
    const Spot& position) {
    std::vector<Plane> result;
    for (int i = 0; i < image.plane_count(); ++i) {
        result.push_back(cut_region_of_interest(*optics[i], image.plane(i), position));
    }
    return result;
}


}
}

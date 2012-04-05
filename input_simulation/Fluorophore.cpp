#define LOCPREC_FLUOROPHORE_CPP
#include <dStorm/debug.h>
#include "Fluorophore.h"

#include "pixelatedBessel.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf.h>

#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>

#include <iostream>
#include <iomanip>

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <dStorm/traits/Projection.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/threed_info/equifocal_plane.h>

using namespace std;

namespace input_simulation {

#define NAME "Fluorophore"
#define DESC "Fluorophore characteristics"

_FluorophoreConfig::_FluorophoreConfig() 
: simparm::Set(NAME, DESC),
  countsPerPhoton("ADCountsPerPhoton", "Camera A/D counts per photon", 16),
  averageActivationTime("AverageActivationTime", "Average time a deactivated "
                        "fluorophore stays deactivated", 2),
  averageDeactivationTime("AverageDeactivationTime", "Average time an activated "
                          "fluorophore stays activated", 0.1),
  photonEmittanceRate("PhotonEmittanceRate", "Photons a fluorophore emits per second",
                      10000),
  numerical_aperture("NumericalAperture", "Simulated numerical aperture", 0.8),
  refractive_index("RefractiveIndex", "Refractive index of lens medium", 1.3),
  wavelength("Wavelength", "Detection wavelength", 500 * boost::units::si::nanometre)
{
}

void _FluorophoreConfig::registerNamedEntries() {
    push_back(countsPerPhoton );
    push_back(averageActivationTime);
    push_back(averageDeactivationTime);
    push_back(photonEmittanceRate);
    push_back(numerical_aperture);
    push_back(refractive_index);
    push_back( wavelength );
}

void Fluorophore::initTimes( const FluorophoreConfig& config ) {
    photonWeight = config.countsPerPhoton();
    av_t_off = config.averageActivationTime() * si::second;
    av_t_on = config.averageDeactivationTime() * si::second;
    photonRate = config.photonEmittanceRate() / boost::units::si::second;
}

Fluorophore::Fluorophore(const Position& pos, int/* noImages*/,
                         const FluorophoreConfig& config,
                         const dStorm::input::Traits< dStorm::engine::ImageStack >& optics,
                         const int fluorophore_index)
: pos(pos), isOn(false), restTime(0)
{
    if ( config.numerical_aperture() >= config.refractive_index() )
        throw std::runtime_error("Simulated numerical aperture is higher "
            "than refractive index. Please reconsider.");
    const double 
        alpha = asin( config.numerical_aperture() / config.refractive_index() ),
        complete_bessel_integral_value = 0.7 * M_PI * alpha * alpha 
            + 0.3 * M_PI * sin( alpha ) * sin( alpha );
    initTimes(config);
    //history.resize(noImages, false);


    planes.resize( optics.plane_count() );
    DEBUG("Generating fluorophore at " << pos.transpose());
    for (int i = 0; i < optics.plane_count(); ++i) {
        DEBUG("Generating plane " << i << " with z position " << *optics.optics(i).z_position);
        Plane& p = planes[i];
        const dStorm::traits::Optics& o = optics.optics(i);
        /* This size of the fluorophore is a safe initial guess - we might need more pixels,
        * and this case is detected by the loop below. */
        p.range[0] = p.range[1] = 4 * camera::pixel;
        p.densities = Eigen::MatrixXd::Constant(p.range[0].value()*2+1, p.range[1].value()*2+1, -1);
        Position plane_pos = pos;
        plane_pos[2] = equifocal_plane( *o.depth_info() );
        p.pixel = optics.plane(i).projection().nearest_point_in_image_space(plane_pos.head<2>());

        DEBUG("Position of fluorophore is " << pos.transpose() << " with center in plane " 
            << p.pixel.transpose());

        BesselFunction bessel( optics.plane(i), pos, 
            config.numerical_aperture(), config.refractive_index(),
            quantity<si::length>( config.wavelength() ),
            optics.plane(i).projection().pixel_size( p.pixel ) );

        const PixelIndex one_pixel = 1 * camera::pixel;
        double total = 0, delta = 0.80;
        BesselFunction::Subpixel to_detect;

        while ( true /* until total area under PSF has been increased by less than delta */ ) {
            for (PixelIndex dx = -p.range[0]; dx <= +p.range[0]; dx += one_pixel) {
                for (PixelIndex dy = -p.range[1]; dy <= +p.range[1]; dy+= one_pixel) {
                    int mx = (dx+p.range[0]) / camera::pixel, my = (dy+p.range[1]) / camera::pixel;
                    if ( p.densities(mx,my) > -0.1 ) continue;
                    to_detect << (p.pixel[0]+dx), (p.pixel[1]+dy);
                    double val = bessel.integrate( to_detect );
                    //DEBUG( "At pixel " << to_detect.transpose() << " have intensity " << val );
                    total += val;
                    p.densities( mx , my ) = val;
                }
            }
            //DEBUG( std::setprecision(10) << "Total increased to " << total );
            
            DEBUG("Integration yielded " << total << " of " << complete_bessel_integral_value);
            if ( total >= delta * complete_bessel_integral_value || p.range[0] >= 20 * camera::pixel )
                break;
            else {
                /* Extend by one pixel */
                p.range[0] += one_pixel;
                p.range[1] += one_pixel;
                DEBUG("Extending generation plane to " << p.range[0] << " " << p.range[1] << " at total " << total);
                Eigen::MatrixXd new_densities = Eigen::MatrixXd::Constant
                    (p.range[0].value()*2+1, p.range[1].value()*2+1, -1);
                new_densities.fill(-1);
                new_densities.block( 1, 1, p.densities.rows(), p.densities.cols() ) = p.densities;
                p.densities = new_densities;
            }
        }
        p.densities *= optics.optics(i).transmission_coefficient( fluorophore_index ) / complete_bessel_integral_value;
#if 0
        if ( i == 0 ) {
            for (int j = -p.range[0].value(); j <= p.range[0].value(); ++j) {
                for (int k = -p.range[1].value(); k <= p.range[1].value(); ++k)
                std::cout << pos[2].value() << " " << j << " " << k << " " << p.densities(j+p.range[0].value(), k+p.range[1].value() ) << "\n";
                std::cout << "\n";
            }
        }
#endif
    }
}

Fluorophore::~Fluorophore() {
}

int Fluorophore::glareInImage(gsl_rng *rng, 
         dStorm::engine::ImageStack &targetImage, int imNum,
         quantity<si::time> integrationTime) 
{
    //history[imNum] = 0;
    if (imNum == 0) /* Do initialization */ {
        double active_prob = (av_t_on / (av_t_off + av_t_on));
        isOn = gsl_rng_uniform(rng) < active_prob;
        restTime = remaining_time_in_current_state(rng);
    }

    unsigned int sum_of_photons = 0;
    const PixelIndex one_pixel = 1 * camera::pixel;
    while (integrationTime > 0 * si::seconds) {
        if (isOn) {
            quantity<si::time> glareTime = min(restTime, integrationTime);

            double totalPhotons = glareTime * photonRate;
            double photonPart;
            unsigned int photonCount, photonSum = 0;

            for ( int plane = 0; plane < targetImage.plane_count(); ++plane ) {
                dStorm::engine::Image2D::Position pos;
                Plane& p = planes[plane];
                for (PixelIndex x = -p.range[0]; x <= p.range[0]; x += one_pixel) {
                    pos.x() = x+p.pixel[0]; 
                    for (PixelIndex y = -p.range[1]; y <= p.range[1]; 
                        y+=one_pixel) 
                    {
                        pos.y() = y+p.pixel[1]; 
                        if ( ! targetImage.plane(plane).contains(pos) )
                            continue;

                        photonPart = p.densities( (x+p.range[0])/camera::pixel,
                                                (y+p.range[1])/camera::pixel );
                        photonCount = gsl_ran_binomial(rng, photonPart,
                                                totalPhotons);
                        photonSum += photonCount;
                        targetImage.plane(plane)(pos)
                            += photonWeight * photonCount;
                    }
                }
            }

            sum_of_photons += photonSum;
            //history[imNum] = photonSum;
        }

        if (restTime > integrationTime) {
            restTime -= integrationTime;
            integrationTime = 0;
        } else {
            integrationTime -= restTime;
            isOn = ! isOn;
            restTime = remaining_time_in_current_state(rng);
        }
    }
    return sum_of_photons;
}

quantity<si::time>
Fluorophore::remaining_time_in_current_state(gsl_rng *rng)
{
    return quantity<si::time>::from_value(
            gsl_ran_exponential(rng, 
                ((isOn) ? av_t_on : av_t_off).value()) );
}

void Fluorophore::recenter( Position np, const dStorm::traits::Projection& optics )
{
    assert( int(planes.size()) == 1 );
    pos = np;
    planes[0].pixel = optics.nearest_point_in_image_space(np.head<2>());
}

std::ostream& operator<<(std::ostream& o, const Fluorophore& f) {
    throw std::logic_error("Fluorophore writing and reading needs to be reimplemented.");
}

}

template <typename Unit, typename Value>
std::istream& operator>>(std::istream& i, 
                         boost::units::quantity<Unit,Value>& v)
{
    Value f;
    i >> f;
    v = boost::units::quantity<Unit,Value>::from_value(f);
    return i;
}

template <typename EigenObject>
std::istream& operator>>(std::istream& i, 
                         Eigen::MatrixBase<EigenObject>& m) {
    for (int r = 0; r < m.rows(); r++)
      for (int c = 0; c < m.cols(); c++)
        i >> m(r,c);
    return i;
}

namespace input_simulation {

Fluorophore::Fluorophore(std::istream& i, const FluorophoreConfig& c) {
    throw std::logic_error("Fluorophore writing and reading needs to be reimplemented.");
    initTimes(c);
}

}

#include <simparm/BoostUnits.hh>
#include "fwd.h"
#include <dStorm/output/Output.h>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <iostream>
#include <fstream>
#include <memory>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/units/microlength.h>
#include <dStorm/units/nanolength.h>

#include <boost/units/cmath.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/weighted_mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>

using namespace boost::accumulators;

namespace dStorm {
namespace calibrate_3d {
namespace sigma_curve {

class Configuration : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
        push_back( object_size );
        push_back( wavelength_correction );
        push_back( step_size );
        push_back( sigma );
    }
  public:
    output::BasenameAdjustedFileEntry outputFile;
    simparm::Entry< quantity<si::nanolength> > object_size, step_size, sigma;
    simparm::Entry< double > wavelength_correction;
    Configuration()
        : simparm::Object("SigmaCurve", "3D PSF width calibration table"),
          outputFile("ToFile", "Calibration output file", "-sigma-table.txt"),
          object_size("ObjectSize", "FWHM correction for object size", 0 * si::nanometre),
          step_size("StepSize", "Calibration point distance", 150 * si::nanometre),
          sigma("SmoothingFactor", "Smoothing Gaussian's sigma", 35 * si::nanometre),
          wavelength_correction("WavelengthCorrection", "WavelengthCorrectionFactor", 1)
        {}

    bool can_work_with(output::Capabilities cap) { 
        return true; 
    }
};

class Output : public output::OutputObject {
private:
    class SigmaEstimate {
        accumulator_set<double, stats<tag::weighted_mean(lazy)>, double> sigmas_in_nm[2];
        quantity<si::length> center_, sigma;
    public:
        SigmaEstimate( quantity<si::length> center, quantity<si::length> sigma )
            : center_(center), sigma(sigma) {}
        void add( const Localization& l ) {
            double distance_weight = exp( -0.5 * pow<2>((l.position().z() - center_) / sigma) );
            for (int i = 0; i < 2; ++i)
                sigmas_in_nm[i]( 
                    sqrt( l.fit_covariance_matrix()(i,i) ) / (1E-9f * si::meter),
                    weight = distance_weight * l.amplitude().value() );
        }
        quantity<si::length> average_sigma(int dim) const
            { return weighted_mean( sigmas_in_nm[dim] ) * 1E-9 * si::meter; }
        quantity<si::length> center() const { return center_; }
    };
    typedef std::map< int, SigmaEstimate > Slots;
    Slots checkpoints;
    const Configuration config;

    float z_step( const Localization& l, double distance ) const {
         return (quantity<si::nanolength>(l.position().z()) + distance * config.sigma()) / config.step_size();
    }
    quantity<si::length> correct_for_size( quantity<si::length> v) const
    {
        return (v - quantity<si::length>(config.object_size() / 2.35)) * config.wavelength_correction();
    }
    void store_results_( bool success ) {
        if ( success ) {
            std::ofstream o ( config.outputFile().c_str() );
            for ( Slots::const_iterator i = checkpoints.begin(); i != checkpoints.end(); ++i )
                o << quantity<si::nanolength>( i->second.center() ).value() << " "
                  << quantity<si::microlength>( correct_for_size(i->second.average_sigma(0)) ).value() << " "
                  << quantity<si::microlength>( correct_for_size(i->second.average_sigma(1)) ).value() << "\n";
        }
    }

public:
    typedef simparm::Structure<Configuration> Config;

    Output(const Config &c) 
        : OutputObject(c.getName(), c.getDesc()),
          config(c) {}
    Output* clone() const { throw std::runtime_error(getDesc() + " cannot be copied"); }

    RunRequirements announce_run(const RunAnnouncement&) {
        checkpoints.clear();
        return RunRequirements();
    }
    AdditionalData announceStormSize(const Announcement &a) {
        if ( ! a.covariance_matrix().is_given(0,0) )
            throw std::runtime_error("PSF width in X is not given for sigma curve generation");
        if ( ! a.covariance_matrix().is_given(1,1) )
            throw std::runtime_error("PSF width in Y is not given for sigma curve generation");
        return AdditionalData(); 
    }
    void receiveLocalizations(const EngineResult& er) {
        for ( EngineResult::const_iterator i = er.begin(); i != er.end(); ++i) {
            int lower_bound = ceil( z_step( *i, -4 ) );
            int upper_bound = floor( z_step( *i, +4 ) );
            for ( int bin = lower_bound; bin <= upper_bound; ++bin ) {
                Slots::value_type default_content( 
                    bin, 
                    SigmaEstimate(
                        double(bin) * quantity<si::length>(config.step_size()), 
                        quantity<si::length>(config.sigma())) );
                SigmaEstimate& e = checkpoints.insert( default_content ).first->second;
                e.add( *i );
            }
        }
    }

};

std::auto_ptr<output::OutputSource> make_output_source() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<Output>() );
}

}
}
}

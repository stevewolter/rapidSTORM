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

#include "calibrate_3d/ZTruth.h"

using namespace boost::accumulators;

namespace dStorm {
namespace calibrate_3d {
namespace sigma_curve {

class Configuration : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
        push_back( filter_ );
        push_back( new_z_ );
        push_back( step_size );
        push_back( sigma );
    }
  public:
    output::BasenameAdjustedFileEntry outputFile;
    simparm::StringEntry filter_, new_z_;
    simparm::Entry< quantity<si::nanolength> > step_size, sigma;
    Configuration()
        : simparm::Object("SigmaCurve", "3D PSF width calibration table"),
          outputFile("ToFile", "Calibration output file", "-sigma-table.txt"),
          filter_("Filter", "Filter expression for usable spots"),
          new_z_("TrueZ", "Expression for true Z value"),
          step_size("StepSize", "Calibration point distance", 150 * si::nanometre),
          sigma("SmoothingFactor", "Smoothing Gaussian's sigma", 35 * si::nanometre)
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
    std::string output_file;
    quantity<si::length> step_width, sigma;
    calibrate_3d::ZTruth z_truth;

    void store_results_( bool success ) {
        if ( success ) {
            std::ofstream o( output_file.c_str() );
            for ( Slots::const_iterator i = checkpoints.begin(); i != checkpoints.end(); ++i )
                o << quantity<si::nanolength>( i->second.center() ).value() << " "
                  << quantity<si::microlength>( i->second.average_sigma(0) ).value() << " "
                  << quantity<si::microlength>( i->second.average_sigma(1) ).value() << "\n";
        }
    }

public:
    typedef simparm::Structure<Configuration> Config;

    Output(const Config &c) 
        : OutputObject(c.getName(), c.getDesc()),
          output_file(c.outputFile()),
          step_width( c.step_size() ),
          sigma( c.sigma() ),
          z_truth( c.filter_(), c.new_z_() ) {}
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
        z_truth.set_meta_info( a );
        return AdditionalData(); 
    }
    void receiveLocalizations(const EngineResult& er) {
        Output::EngineResult ls = er;
        Output::EngineResult::iterator begin = ls.begin(), end = z_truth.calibrate( ls );
        for ( EngineResult::iterator i = ls.begin(); i != end; ++i) {
            i->position().z() = z_truth.true_z( *i );
            int lower_bound = ceil( (i->position().z() - 4.0 * sigma) / step_width );
            int upper_bound = floor( (i->position().z() + 4.0 * sigma) / step_width );
            for ( int bin = lower_bound; bin <= upper_bound; ++bin ) {
                Slots::value_type default_content( bin, SigmaEstimate(double(bin) * step_width, sigma) );
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

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

#include <gsl/gsl_bspline.h>
#include <gsl/gsl_multifit.h>
#include <Eigen/LU>

using namespace boost::accumulators;

namespace dStorm {
namespace calibrate_3d {
namespace sigma_curve {

class Configuration {
  public:
    output::BasenameAdjustedFileEntry outputFile;
    simparm::Entry< unsigned int > step_number;
    static std::string get_name() { return "SigmaCurve"; }
    static std::string get_description() { return "3D PSF width calibration table"; }
    Configuration()
        : outputFile("ToFile", "Calibration output file", "-sigma-table.txt"),
          step_number("StepNumber", "Number of B spline breakpoints", 10)
    {
        outputFile.helpID = "#SigmaCurve_ToFile";
        step_number.helpID = "#SigmaCurve_StepNumber";
    }
    void attach_ui( simparm::Node& at ) {
        outputFile.attach_ui( at );
        step_number.attach_ui( at );
    }

    bool can_work_with(output::Capabilities cap) { 
        return true; 
    }
};

class Output : public output::Output {
private:
    class SigmaPair {
        quantity<si::length> z;
        quantity<si::length> s[2];
        quantity<camera::intensity> amp;
    public:
        SigmaPair( const Localization& l )
            : z( l.position().z() ), amp( l.amplitude() ) 
            { for (int i = 0; i < 2; ++i) s[i] = l.psf_width()[i] / 2.35f; }
        bool smaller_z( const SigmaPair& o ) const { return z < o.z; }

        double scaled_z() const { return z / (1E-6 * si::meter); }
        double weight() const { return amp / (1E3 * camera::ad_count); }
        double sigma(int dir) const { return s[dir] / (1E-6 * si::meter); }

        static quantity<si::length> from_z( double z ) { return z * 1E-6 * si::meter; }
        static quantity<si::length> from_sigma( double s ) { return s * 1E-6 * si::meter; }
    };
    std::vector< SigmaPair > points;
    const Configuration config;

    void store_results_( bool success ) {
        if ( success ) {
            static const size_t order = 4;
            const size_t n = points.size(), nbreak = config.step_number(), ncoeffs = nbreak - 2 + order;

            boost::shared_ptr<gsl_bspline_workspace> bw
                ( gsl_bspline_alloc(order, nbreak), std::ptr_fun(&gsl_bspline_free) );
            boost::shared_ptr<gsl_vector> B
                ( gsl_vector_alloc(ncoeffs), std::ptr_fun(&gsl_vector_free) );
            double minx = std::min_element( points.begin(), points.end(), 
                    std::mem_fun_ref( &SigmaPair::smaller_z ) )->scaled_z(),
                   maxx = std::max_element( points.begin(), points.end(), 
                    std::mem_fun_ref( &SigmaPair::smaller_z ) )->scaled_z();
            gsl_bspline_knots_uniform( minx, maxx, bw.get() );

            /* construct the fit matrix X */
            boost::shared_ptr<gsl_matrix> X
                ( gsl_matrix_alloc( n, ncoeffs ), std::ptr_fun(&gsl_matrix_free) );
            boost::shared_ptr<gsl_vector> w
                ( gsl_vector_alloc( n ), std::ptr_fun(&gsl_vector_free) );
            boost::shared_ptr<gsl_vector> sigma[2];
            for (int i = 0; i < 2; ++i)
                sigma[i] = boost::shared_ptr<gsl_vector>( gsl_vector_alloc(n), std::ptr_fun(&gsl_vector_free) );
            for ( std::vector< SigmaPair >::const_iterator i = points.begin(); i != points.end(); ++i ) {
                const int row = i - points.begin();
                gsl_vector_view view = gsl_matrix_row( X.get(), row );
                gsl_bspline_eval(i->scaled_z(), &view.vector, bw.get());
                gsl_vector_set( w.get(), row, i->weight() );
                for (int dir = 0; dir < 2; ++dir)
                    gsl_vector_set( sigma[dir].get(), row, i->sigma(dir) );
            }

            boost::shared_ptr<gsl_multifit_linear_workspace> mw
                ( gsl_multifit_linear_alloc(n, ncoeffs), std::ptr_fun(&gsl_multifit_linear_free) );
            boost::shared_ptr<gsl_matrix> cov[2];
            boost::shared_ptr<gsl_vector> c[2];
            for (int dir = 0; dir < 2; ++dir) {
                cov[dir] = boost::shared_ptr<gsl_matrix>( gsl_matrix_alloc(ncoeffs, ncoeffs), std::ptr_fun(&gsl_matrix_free) );
                c[dir] = boost::shared_ptr<gsl_vector>( gsl_vector_alloc(ncoeffs), std::ptr_fun(&gsl_vector_free) );
                double chisq;
                gsl_multifit_wlinear(X.get(), w.get(), sigma[dir].get(), c[dir].get(), cov[dir].get(), &chisq, mw.get());
            }

            std::ofstream o ( config.outputFile().c_str() );
            for ( size_t i = 0; i < nbreak; ++i ) {
                double xi = gsl_vector_get( bw->knots, i+order-1 );
                double yi[2], yerr[2];
                gsl_bspline_eval(xi, B.get(), bw.get());
                for (int dir = 0; dir < 2; ++dir)
                    gsl_multifit_linear_est(B.get(), c[dir].get(), cov[dir].get(), yi+dir, yerr+dir);
                o << double(SigmaPair::from_z( xi ) / (1E-9 * si::meter)) << " " 
                  << double(SigmaPair::from_sigma( yi[0] ) / (1E-6 * si::meter)) << " "
                  << double(SigmaPair::from_sigma( yi[1] ) / (1E-6 * si::meter)) << "\n";
            }
        }
    }

public:
    Output(const Configuration &c) 
        : config(c) {}

    RunRequirements announce_run(const RunAnnouncement&) {
        return RunRequirements();
    }
    AdditionalData announceStormSize(const Announcement &a) {
        if ( ! a.position().is_given.z() )
            throw std::runtime_error("Z ground truth is not given for sigma curve generation");
        if ( ! a.psf_width().is_given[0] )
            throw std::runtime_error("PSF width in X is not given for sigma curve generation");
        if ( ! a.psf_width().is_given[1] )
            throw std::runtime_error("PSF width in Y is not given for sigma curve generation");
        return AdditionalData(); 
    }
    void receiveLocalizations(const EngineResult& er) {
        std::copy( er.begin(), er.end(), std::back_inserter( points ) );
    }

};
std::auto_ptr<output::OutputSource> make_output_source() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<Configuration,Output>() );
}

}
}
}

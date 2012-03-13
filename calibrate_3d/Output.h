#ifndef DSTORM_CALIBRATE_3D_OUTPUT_H
#define DSTORM_CALIBRATE_3D_OUTPUT_H

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/count.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <boost/optional/optional.hpp>

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/Structure.hh>

#include <dStorm/output/Output.h>
#include <dStorm/traits/resolution_config.h>
#include <dStorm/units/nanolength.h>

#include "Config.h"
#include "ParameterLinearizer.h"

#include "expression/Parser.h"
#include "expression/LValue.h"

namespace dStorm {
namespace calibrate_3d {

class TrueZ;

class Output : public output::OutputObject {
  private:
    expression::Parser parser;
    TrueZ *new_z_variable;
    input::Traits<Localization> localization_traits;
    boost::shared_ptr< engine::InputTraits > initial_traits;
    ParameterLinearizer linearizer;
    Engine* engine;
    std::auto_ptr< expression::source::LValue > filter, new_z_expression;
    dStorm::traits::resolution::Config result_config;

    boost::thread calibration_thread;
    boost::mutex mutex;
    boost::condition new_job, value_computed;
    boost::optional<double> position_value;
    boost::optional< std::auto_ptr<engine::InputTraits> > trial_position;
    bool have_set_traits_myself, terminate, fitter_finished;

    typedef boost::accumulators::accumulator_set< 
            double,
            boost::accumulators::stats< 
                boost::accumulators::tag::variance,
                boost::accumulators::tag::count > >
        Variance;
    Variance delta_z;

    const Config_ config;
    simparm::Entry<double> current_volume;
    simparm::Entry< quantity<si::nanolength> > residuals;

    std::vector<int> variable_map;

    void run_finished_(const RunFinished&);
    void run_fitter();
    double evaluate_function( const gsl_vector *x );
    static double gsl_callback( const gsl_vector * x, void * params )
        { return static_cast<Output*>(params)->evaluate_function(x); }

  public:
    typedef simparm::Structure<Config_> Config;

    Output(const Config &config);
    Output *clone() const { throw std::logic_error("Not implemented"); }
    ~Output();

    AdditionalData announceStormSize(const Announcement &);
    RunRequirements announce_run(const RunAnnouncement&);
    void receiveLocalizations(const EngineResult&);
    void store_results() {}

    const char *getName() { return "Calibrate 3D"; }
};

}
}

#endif

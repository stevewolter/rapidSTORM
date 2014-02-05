#ifndef DSTORM_CALIBRATE_3D_OUTPUT_H
#define DSTORM_CALIBRATE_3D_OUTPUT_H

#include <boost/smart_ptr/scoped_ptr.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <boost/optional/optional.hpp>

#include <boost/units/systems/si/area.hpp>
#include <boost/units/quantity.hpp>

#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include <simparm/Object.h>
#include <simparm/Entry.h>

#include <dStorm/output/Output.h>
#include <dStorm/traits/optics_config.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/stack_realign.h>

#include "calibrate_3d/Config.h"
#include "calibrate_3d/ParameterLinearizer.h"

#include "expression/Parser.h"
#include "expression/LValue.h"

namespace dStorm {
namespace calibrate_3d {

class Output : public output::Output {
  private:
    boost::scoped_ptr<ZTruth> z_truth;
    boost::shared_ptr< engine::InputTraits > initial_traits;
    ParameterLinearizer linearizer;
    Engine* engine;
    dStorm::traits::MultiPlaneConfig result_config;

    boost::thread calibration_thread;
    boost::mutex mutex;
    boost::condition new_job, value_computed;
    boost::optional<double> position_value;
    boost::optional< std::auto_ptr<engine::InputTraits> > trial_position;
    bool have_set_traits_myself, terminate, fitter_finished;

    int found_spots;
    quantity<si::area> squared_errors;

    const Config config;
    simparm::Entry<double> current_volume;
    simparm::Entry< quantity<si::nanolength> > residuals;

    std::vector<int> variable_map;

    void run_finished_(const RunFinished&);
    DSTORM_REALIGN_STACK void run_fitter();
    double evaluate_function( const gsl_vector *x );
    static double gsl_callback( const gsl_vector * x, void * params )
        { return static_cast<Output*>(params)->evaluate_function(x); }

    void attach_ui_( simparm::NodeHandle );

  public:
    Output(const Config &config);
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

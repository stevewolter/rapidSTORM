#ifndef DSTORM_EXPRESSION_SIMPLEFILTERS_H
#define DSTORM_EXPRESSION_SIMPLEFILTERS_H

#include "simparm/Eigen_decl.h"
#include "simparm/BoostUnits.h"
#include <boost/units/Eigen/Core>
#include "simparm/Eigen.h"
#include "simparm/BoostOptional.h"
#include "simparm/Entry.h"
#include "CommandLine.h"
#include "units/nanolength.h"
#include "localization/Traits.h"

namespace dStorm {
namespace expression {

struct SimpleFilters 
{
    SimpleFilters();
    SimpleFilters(const SimpleFilters&);
    void set_manager( config::ExpressionManager * manager );
    void set_visibility( const input::Traits<Localization>& );
    void attach_ui( simparm::NodeHandle );

    typedef boost::units::divide_typeof_helper< 
        boost::units::power10< boost::units::si::length, -12 >::type,
        boost::units::camera::time 
    >::type ShiftSpeed;
  private:
    config::ExpressionManager * manager;
    simparm::Entry< boost::optional< boost::units::quantity< boost::units::camera::intensity > > >
        lower_amplitude;
    simparm::Entry< boost::optional< Eigen::Matrix< boost::units::quantity<ShiftSpeed,float>, 3, 1, Eigen::DontAlign> > >
        drift_correction;
    simparm::Entry< float > two_kernel_improvement;
    simparm::BaseAttribute::ConnectionStore listening[3];

    void publish_amp();
    void publish_drift_correction();
    void publish_tki();
};

}
}

#endif

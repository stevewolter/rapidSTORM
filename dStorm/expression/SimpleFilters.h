#ifndef DSTORM_EXPRESSION_SIMPLEFILTERS_H
#define DSTORM_EXPRESSION_SIMPLEFILTERS_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <dStorm/units_Eigen_traits.h>
#include <simparm/Eigen.hh>
#include <simparm/BoostOptional.hh>
#include <simparm/Entry.hh>
#include "CommandLine.h"
#include <dStorm/units/nanolength.h>

namespace dStorm {
namespace expression {

struct SimpleFilters 
: public simparm::Listener
{
    SimpleFilters( boost::shared_ptr<variable_table> variables );
    SimpleFilters(const SimpleFilters&);
    void set_manager( config::ExpressionManager * manager );

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
    boost::shared_ptr<variable_table> variables;
    void operator()(const simparm::Event&);
    void publish_amp();
    void publish_drift_correction();
};

}
}

#endif

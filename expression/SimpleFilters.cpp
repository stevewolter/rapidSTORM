#include "expression/SimpleFilters.h"
#include "expression/LValue.h"
#include <boost/bind/bind.hpp>
#include <boost/units/cmath.hpp>

using namespace boost::units;

namespace boost {
namespace units {

std::string name_string( divide_typeof_helper< power10<si::length, -12>::type, camera::time >::type ) 
    { return "picometer per frame"; }
std::string symbol_string( divide_typeof_helper< power10<si::length, -12>::type, camera::time >::type ) 
    { return "pm/fr"; }

}
}

namespace dStorm {
namespace expression {

class LowerAmplitude : public source::LValue {
    typedef quantity< camera::intensity, float > Value;
    Value value;
  public:
    typedef bool result_type;
    LowerAmplitude( const quantity< camera::intensity >& v )
        : value(v) {}
    LowerAmplitude* clone() const { return new LowerAmplitude(*this); }
    iterator evaluate( 
        const variable_table&, const input::Traits<Localization>&, 
        iterator begin, iterator end ) const 
    {
        return std::remove_if( begin, end, *this );
    }
    void announce( const variable_table&, input::Traits<Localization>& traits ) const {
        Value bound = value;
        if ( traits.amplitude().range().first.is_initialized() )
            bound = std::max( Value(*traits.amplitude().range().first), value );
        traits.amplitude().range().first = bound;
    }

    bool operator()( const Localization& l ) { return l.amplitude() < value; }
};

class DriftCorrection : public source::LValue {
    typedef quantity< SimpleFilters::ShiftSpeed, float > Scalar;
    typedef Eigen::Matrix< Scalar, 3, 1, Eigen::DontAlign > Value;
    Value value;
  public:
    typedef void result_type;
    DriftCorrection( const Value& v ) : value(v) {}
    DriftCorrection* clone() const { return new DriftCorrection(*this); }
    iterator evaluate( 
        const variable_table&, const input::Traits<Localization>&, 
        iterator begin, iterator end ) const 
    {
        std::for_each( begin, end, *this );
        return end;
    }
    void announce( const variable_table&, input::Traits<Localization>& traits ) const {}

    void operator()( Localization& l ) { 
        l.position_x() += samplepos::Scalar(l.frame_number() * value.x());
        l.position_y() += samplepos::Scalar(l.frame_number() * value.y());
        l.position_z() += samplepos::Scalar(l.frame_number() * value.z());
    }

};

class TwoKernelImprovement : public source::LValue {
    typedef float Value;
    Value value;
  public:
    typedef bool result_type;
    TwoKernelImprovement( const Value& v ) : value(v) {}
    TwoKernelImprovement* clone() const { return new TwoKernelImprovement(*this); }
    iterator evaluate( 
        const variable_table&, const input::Traits<Localization>&, 
        iterator begin, iterator end ) const 
    {
        return std::remove_if( begin, end, *this );
    }
    void announce( const variable_table&, input::Traits<Localization>& traits ) const { }

    bool operator()( const Localization& l ) { return l.two_kernel_improvement() > value; }
};

SimpleFilters::SimpleFilters()
: manager(NULL), 
  lower_amplitude("LowerAmplitudeThreshold", "Minimum localization strength", boost::optional< boost::units::quantity< boost::units::camera::intensity > >() ) ,
  drift_correction("LinearDriftCorrection", "Linear drift correction", boost::optional< Eigen::Matrix< boost::units::quantity<ShiftSpeed,float>, 3, 1, Eigen::DontAlign> >() ),
  two_kernel_improvement("TwoKernelImprovement", "Maximum two kernel improvement", 1)
{
    lower_amplitude.setHelpID( "ExpressionFilter_LowerAmplitudeThreshold" );
    drift_correction.setHelpID( "ExpressionFilter_LinearDriftCorrection" );
    two_kernel_improvement.setHelpID( "ExpressionFilter_TwoKernelImprovement" );

    two_kernel_improvement.min  = 0;
    two_kernel_improvement.max  = 1;
    two_kernel_improvement.increment  = 0.01;
}

SimpleFilters::SimpleFilters( const SimpleFilters& o )
: manager(NULL), 
  lower_amplitude(o.lower_amplitude) ,
  drift_correction(o.drift_correction),
  two_kernel_improvement(o.two_kernel_improvement)
{
}

void SimpleFilters::set_manager(config::ExpressionManager* m) {
    manager = m;
    publish_amp();
    publish_drift_correction();
    publish_tki();
}

void SimpleFilters::attach_ui(simparm::NodeHandle at) {
    listening[0] = lower_amplitude.value.notify_on_value_change( 
        boost::bind( &SimpleFilters::publish_amp, this ) );
    listening[1] = drift_correction.value.notify_on_value_change( 
        boost::bind( &SimpleFilters::publish_drift_correction, this ) );
    listening[2] = two_kernel_improvement.value.notify_on_value_change( 
        boost::bind( &SimpleFilters::publish_tki, this ) );

    lower_amplitude.attach_ui(at);
    drift_correction.attach_ui(at);
    two_kernel_improvement.attach_ui(at);
}

void SimpleFilters::publish_amp() 
{
    if ( !manager ) return;

    std::auto_ptr< source::LValue > rv;
    if ( lower_amplitude().is_initialized() )
        rv.reset( new LowerAmplitude(*lower_amplitude()) );
    manager->expression_changed("SimpleFilters.Amp", rv);
}

void SimpleFilters::publish_drift_correction() 
{
    if ( !manager ) return;

    std::auto_ptr< source::LValue > rv;
    if ( drift_correction().is_initialized() )
        rv.reset( new DriftCorrection(*drift_correction()) );
    manager->expression_changed("SimpleFilters.DriftCorrection", rv);
}

void SimpleFilters::publish_tki() 
{
    if ( !manager ) return;

    std::auto_ptr< source::LValue > rv;
    if ( two_kernel_improvement() <= 0.999999 )
        rv.reset( new TwoKernelImprovement(two_kernel_improvement()) );
    manager->expression_changed("SimpleFilters.TKI", rv);
}

void SimpleFilters::set_visibility( const input::Traits<Localization>& a ) {
    two_kernel_improvement.set_visibility ( a.two_kernel_improvement().is_given );
}

}
}

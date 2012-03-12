#include "SimpleFilters.h"
#include "LValue.h"
#include <simparm/Entry_Impl.hh>
#include "localization_variable_impl.h"
#include <dStorm/traits/tags.h>
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
        for (int i = 0; i < l.position().rows(); ++i) {
            l.position()[i] += samplepos::Scalar(l.frame_number() * value[i]);
        }
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
: simparm::Listener(simparm::Event::ValueChanged),
  manager(NULL), 
  lower_amplitude("LowerAmplitudeThreshold", "Minimum localization strength") ,
  drift_correction("LinearDriftCorrection", "Linear drift correction"),
  two_kernel_improvement("TwoKernelImprovement", "Maximum two kernel improvement", 1)
{
    two_kernel_improvement.min  = 0;
    two_kernel_improvement.max  = 1;
    two_kernel_improvement.increment  = 0.01;

    receive_changes_from( lower_amplitude.value );
    receive_changes_from( drift_correction.value );
    receive_changes_from( two_kernel_improvement.value );
}

SimpleFilters::SimpleFilters( const SimpleFilters& o )
: simparm::Listener(simparm::Event::ValueChanged),
  manager(o.manager), 
  lower_amplitude(o.lower_amplitude) ,
  drift_correction(o.drift_correction),
  two_kernel_improvement(o.two_kernel_improvement)
{
    receive_changes_from( lower_amplitude.value );
    receive_changes_from( drift_correction.value );
    receive_changes_from( two_kernel_improvement.value );
}

void SimpleFilters::set_manager(config::ExpressionManager* m) {
    manager = m;
    if ( m ) m->getNode().push_back( lower_amplitude );
    if ( m ) m->getNode().push_back( drift_correction );
    if ( m ) m->getNode().push_back( two_kernel_improvement );
    publish_amp();
    publish_drift_correction();
    publish_tki();
}

void SimpleFilters::operator()(const simparm::Event& e)
{
    if ( &e.source == &lower_amplitude.value )
        publish_amp();
    if ( &e.source == &drift_correction.value )
        publish_drift_correction();
    if ( &e.source == &two_kernel_improvement.value )
        publish_tki();
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
    two_kernel_improvement.viewable = ( a.two_kernel_improvement().is_given );
}

}
}

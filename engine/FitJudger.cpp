#include "engine/FitJudger.h"
#include "engine/FitJudgerFactory.h"
#include <simparm/Object.h>
#include <simparm/Entry.h>
#include "engine/InputPlane.h"

namespace dStorm {
namespace engine {

class FixedThresholdJudger : public FitJudger {
    double local_threshold;
public:
    FixedThresholdJudger( double local_threshold ) : local_threshold(local_threshold) {}
    FixedThresholdJudger* clone() const { return new FixedThresholdJudger(*this); }
    bool is_above_background( double signal_integral_in_photons, double background_in_photons ) const
        { return signal_integral_in_photons > local_threshold; }
};

class FixedThresholdJudgerFactory : public FitJudgerFactory {
    simparm::Object name_object;
    simparm::Entry< boost::units::quantity<
            boost::units::camera::intensity, float> > amplitude_threshold;
public:
    FixedThresholdJudgerFactory() 
    :   name_object("FixedThreshold"),
        amplitude_threshold("AmplitudeThreshold", 1000 * boost::units::camera::ad_count) 
    {
        name_object.set_user_level( simparm::Beginner );
        amplitude_threshold.set_user_level( simparm::Beginner );
    }
    FixedThresholdJudgerFactory* clone() const { return new FixedThresholdJudgerFactory(*this); }
    std::auto_ptr< FitJudger > make_fit_judger( const InputPlane& p ) const {
        quantity<camera::intensity> photon = 
                p.optics.photon_response.get_value_or( 1 * camera::ad_count );
        return std::auto_ptr< FitJudger >( new FixedThresholdJudger( amplitude_threshold() / photon ) );
    }
    void attach_ui( simparm::NodeHandle n ) {
        simparm::NodeHandle r = name_object.attach_ui( n );
        amplitude_threshold.attach_ui( r );
    }
    std::string getName() const { return name_object.getName(); }
    void set_variables( output::Basename& ) const {}
};

std::auto_ptr< FitJudgerFactory > make_fixed_threshold_judger() {
    return std::auto_ptr< FitJudgerFactory >( new FixedThresholdJudgerFactory() );
}

class SquareRootRatioJudger : public FitJudger {
    double ratio;
public:
    SquareRootRatioJudger( double ratio ) : ratio(ratio) {}
    SquareRootRatioJudger* clone() const { return new SquareRootRatioJudger(*this); }
    bool is_above_background( double signal_integral_in_photons, double background_in_photons ) const
        { return signal_integral_in_photons / sqrt( background_in_photons ) > ratio; }
};

class SquareRootRatioJudgerFactory : public FitJudgerFactory {
    simparm::Object name_object;
    simparm::Entry< double > snr;
public:
    SquareRootRatioJudgerFactory() 
    :   name_object("SquareRootRatio"),
        snr("SNR", 30) 
    {
        name_object.set_user_level( simparm::Intermediate );
    }

    SquareRootRatioJudgerFactory* clone() const { return new SquareRootRatioJudgerFactory(*this); }

    std::auto_ptr< FitJudger > make_fit_judger( const InputPlane& ) const {
        return std::auto_ptr< FitJudger >( new SquareRootRatioJudger( snr() ) );
    }
    void attach_ui( simparm::NodeHandle n ) {
        simparm::NodeHandle r = name_object.attach_ui( n );
        snr.attach_ui( r );
    }
    std::string getName() const { return name_object.getName(); }
    void set_variables( output::Basename& ) const {}
};

std::auto_ptr< FitJudgerFactory > make_square_root_ratio_judger() {
    return std::auto_ptr< FitJudgerFactory >( new SquareRootRatioJudgerFactory() );
}

#if 0
            quantity<camera::intensity> photon = 
                info.traits.optics(plane)
                    .photon_response.get_value_or( 1 * camera::ad_count );
#endif

#if 0 /* Code for global-relative fit judger */
    if ( config.guess_threshold() ) {
        DEBUG("Guessing input threshold");
        bool have_set_threshold = false;
        for ( int i = 0; i < imProp->plane_count(); ++i ) {
            if ( imProp->plane(i).optics.background_stddev.is_initialized() ) {
                if ( imProp->optics(i).transmission_coefficient(0) > 1E-2 ) {
                    camera_response threshold = 
                        config.threshold_height_factor() * *imProp->optics(i).background_stddev / 
                            imProp->optics(i).transmission_coefficient(0);
                    if ( ! have_set_threshold || config.amplitude_threshold() > threshold )
                        config.amplitude_threshold = threshold;
                    have_set_threshold = true;
                }
            }
        }
        if ( ! have_set_threshold )
            throw std::runtime_error("Amplitude threshold is not set and could not be determined from background noise strength");
        DEBUG("Guessed amplitude threshold " << config.amplitude_threshold());
    } else {
        DEBUG("Using amplitude threshold " << config.amplitude_threshold());
    }
#endif

}
}

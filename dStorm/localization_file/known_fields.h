#ifndef DSTORM_LOCALIZATION_FILE_KNOWN_FIELDS_H
#define DSTORM_LOCALIZATION_FILE_KNOWN_FIELDS_H

#include <cs_units/camera/length.hpp>
#include <cs_units/camera/resolution.hpp>
#include <boost/units/systems/si/base.hpp>

#include <dStorm/Localization.h>
#include <dStorm/input/LocalizationTraits.h>

#include "known_fields_decl.h"

namespace dStorm {
namespace LocalizationFile {
namespace field {
namespace properties {

typedef input::Traits<Localization> Traits;

template <int Dimension>
struct Spatial {
    typedef Localization::Position::Scalar ValueQuantity;
    typedef ValueQuantity::unit_type ValueUnit;
    typedef Traits::Size::Scalar BoundQuantity;
    typedef Traits::Resolution ResolutionField;
    typedef ResolutionField::value_type ResolutionQuantity;

    static const std::string semantic;
    static const bool hasMinField = false,
                        hasMaxField = true,
                        hasResolutionField = true;

    static BoundQuantity& minField( Traits& )
        { throw std::logic_error("No minimum field given."); }
    static BoundQuantity& maxField( Traits& l )
        { return l.size[Dimension]; }
    static ResolutionField& resolutionField
        ( Traits& l ) { return l.resolution; }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.position()[Dimension] = value; }
};

struct Time {
    typedef frame_index ValueQuantity;
    typedef ValueQuantity::unit_type ValueUnit;
    typedef Traits::FrameCountField BoundQuantity;
    typedef simparm::optional<BoundQuantity> BoundField;
    typedef Traits::FrameRateField ResolutionQuantity;
    typedef simparm::optional<ResolutionQuantity>
        ResolutionField;

    static const std::string semantic;
    static const bool hasMinField = false,
                        hasMaxField = true,
                        hasResolutionField = true;

    static BoundField& minField( Traits& )
        { throw std::logic_error("No minimum field given."); }
    static BoundField& maxField( Traits& l )
        { return l.total_frame_count; }
    static ResolutionField& resolutionField
            ( Traits& l ) { return l.speed; }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.setImageNumber( value ); }
};

struct Amplitude {
    typedef Localization::Amplitude ValueQuantity;
    typedef ValueQuantity::unit_type ValueUnit;
    typedef Traits::AmplitudeField SizeQuantity;
    typedef ValueQuantity BoundQuantity;
    typedef simparm::optional<BoundQuantity> BoundField;

    static const std::string semantic;
    static const bool hasMinField = true,
                        hasMaxField = false;

    static BoundField& minField( Traits& l )
        { return l.min_amplitude; }
    static BoundField& maxField( Traits& )
        { throw std::logic_error("No maximum field given."); }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.strength() = value; }
};

struct TwoKernelImprovement {
    typedef boost::units::quantity<
        boost::units::si::dimensionless, float> ValueQuantity;
    typedef ValueQuantity::unit_type ValueUnit;
    typedef int BoundQuantity;
    typedef int BoundField;

    static const std::string semantic;
    static const bool hasMinField = false, hasMaxField = false;

    static BoundField& minField( Traits& l )
        { throw std::logic_error("No minimum field given."); }
    static BoundField& maxField( Traits& )
        { throw std::logic_error("No maximum field given."); }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.two_kernel_improvement() = value; }
};

struct CovarianceMatrix {
    typedef Localization::Matrix ValueQuantity;
    typedef ValueQuantity::Scalar::unit_type ValueUnit;
    typedef int BoundQuantity;
    typedef int BoundField;

    static const std::string semantic;
    static const bool hasMinField = false, hasMaxField = false;

    static BoundField& minField( Traits& l )
        { throw std::logic_error("No minimum field given."); }
    static BoundField& maxField( Traits& )
        { throw std::logic_error("No maximum field given."); }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.fit_covariance_matrix() = value; }
};

}
}
}
}

#endif

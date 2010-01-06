#ifndef DSTORM_LOCALIZATION_FILE_KNOWN_FIELDS_H
#define DSTORM_LOCALIZATION_FILE_KNOWN_FIELDS_H

#include <dStorm/Localization.h>
#include <dStorm/units.h>
#include <dStorm/input/LocalizationTraits.h>

#include "known_fields_decl.h"

namespace dStorm {
namespace LocalizationFile {
namespace field {
namespace properties {

typedef input::Traits<Localization> Traits;

template <int Dimension>
struct Spatial {
    typedef quantity<camera::length> ValueQuantity;
    typedef quantity<camera::pixel_size> ResolutionQuantity;

    static const std::string semantic;
    static const bool hasMinField = false,
                        hasMaxField = true,
                        hasResolutionField = true;

    static ValueQuantity& minField( Traits& )
        { throw std::logic_error("No minimum field given."); }
    static typename Traits::Size::Scalar&
        maxField( Traits& l )
        { return l.size[Dimension]; }
    static Traits::Resolution::Scalar& resolutionField
        ( Traits& l )
        { return l.resolution[Dimension]; }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.position()[Dimension] = value; }
};

struct Time {
    typedef quantity<camera::time, int> ValueQuantity;
    typedef quantity<camera::frame_rate> ResolutionQuantity;
    typedef simparm::optional<ValueQuantity> BoundField;
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
            ( Traits& l )
        { return l.frame_length; }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.setImageNumber( value.value() ); }
};

struct Amplitude {
    typedef quantity<camera::intensity> ValueQuantity;
    typedef simparm::optional<ValueQuantity> BoundField;

    static const std::string semantic;
    static const bool hasMinField = true,
                        hasMaxField = false;

    static BoundField& minField( Traits& l )
        { return l.min_amplitude; }
    static BoundField& maxField( Traits& )
        { throw std::logic_error("No maximum field given."); }
    static void insert( const ValueQuantity& value,
                        Localization& target )
        { target.strength() = value.value(); }
};

}
}
}
}

#endif

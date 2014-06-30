#ifndef DSTORM_LOCALIZATION_FIELDS_H
#define DSTORM_LOCALIZATION_FIELDS_H

#include <boost/mpl/vector.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>

#include "boost/units/systems/camera/time.hpp"
#include "boost/units/systems/camera/intensity.hpp"
#include "boost/units/power10.hpp"
#include "types/fluorophore.h"
#include "types/samplepos.h"
#include "units/nanolength.h"

namespace dStorm {
namespace localization {

template <int Dimension>
struct Position {
    typedef samplepos::Scalar ValueType;
    typedef boost::units::quantity< boost::units::si::nanolength, float > OutputType;

    static std::string get_desc();
    static std::string get_shorthand();

    static const samplepos::Scalar default_value;
};

typedef Position<0> PositionX;
typedef Position<1> PositionY;
typedef Position<2> PositionZ;

template <int Dimension>
struct PositionUncertainty
{
    typedef samplepos::Scalar ValueType;
    typedef boost::units::quantity< boost::units::si::nanolength, float > OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const boost::units::quantity<boost::units::si::length,float> default_value;
};

typedef PositionUncertainty<0> PositionUncertaintyX;
typedef PositionUncertainty<1> PositionUncertaintyY;
typedef PositionUncertainty<2> PositionUncertaintyZ;

struct Amplitude {
    typedef boost::units::quantity< boost::units::camera::intensity, float > ValueType;
    typedef boost::units::quantity< boost::units::camera::intensity, float > OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

template <int Dimension>
struct PSFWidth {
    typedef boost::units::quantity< boost::units::si::length, float > ValueType;
    typedef boost::units::quantity< boost::units::power10< boost::units::si::length, -9 >::type, float > OutputType;
    static std::string get_desc();
    static std::string get_shorthand();
    static const boost::units::quantity< boost::units::si::length, float > default_value;
};

typedef PSFWidth<0> PSFWidthX;
typedef PSFWidth<1> PSFWidthY;

struct TwoKernelImprovement {
    typedef boost::units::quantity<boost::units::si::dimensionless, float> ValueType;
    typedef boost::units::quantity<boost::units::si::dimensionless, float> OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

struct CoefficientOfDetermination {
    typedef boost::units::quantity<boost::units::si::dimensionless, float> ValueType;
    typedef boost::units::quantity<boost::units::si::dimensionless, float> OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

struct FitResidues {
    typedef boost::units::quantity<boost::units::si::dimensionless, double> ValueType;
    typedef boost::units::quantity<boost::units::si::dimensionless, double> OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

struct Fluorophore {
    typedef dStorm::Fluorophore ValueType;
    typedef dStorm::Fluorophore OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

struct LocalBackground {
    typedef boost::units::quantity<boost::units::camera::intensity, float> ValueType;
    typedef boost::units::quantity<boost::units::camera::intensity, float> OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

struct ImageNumber {
    typedef boost::units::quantity<boost::units::camera::time, int> ValueType;
    typedef boost::units::quantity<boost::units::camera::time, int> OutputType;

    static std::string get_desc();
    static std::string get_shorthand();

    static const ValueType default_value;
};

struct Molecule {
    typedef dStorm::Fluorophore ValueType;
    typedef dStorm::Fluorophore OutputType;

    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};


typedef boost::mpl::vector<
    PositionX,
    PositionUncertaintyX,
    PositionY,
    PositionUncertaintyY,
    PositionZ,
    PositionUncertaintyZ,
    ImageNumber,
    Amplitude,
    PSFWidthX,
    PSFWidthY,
    TwoKernelImprovement,
    FitResidues,
    Fluorophore,
    LocalBackground,
    CoefficientOfDetermination,
    Molecule> Fields;

}
}

#endif

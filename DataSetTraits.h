#ifndef DSTORM_DATASET_TRAITS
#define DSTORM_DATASET_TRAITS

#include <boost/optional/optional.hpp>
#include <boost/units/systems/si/time.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/quantity.hpp>
#include <map>
#include <set>
#include <string>

namespace dStorm {

struct FluorophoreTraits {
    int ident;
};

struct DataSetTraits {
    boost::optional< boost::units::quantity<boost::units::si::time,float> > exposure_time;
    typedef std::map<int, boost::optional<std::string> > InfoMap;
    InfoMap infos;
    std::map<int,FluorophoreTraits> fluorophores;

    enum InfoTypes {
        CameraTemperature, OutputAmplifierType, VerticalShiftSpeed, 
        HorizontalShiftSpeed, PreamplifierGain, ExposureTime
    };
};

}

#endif

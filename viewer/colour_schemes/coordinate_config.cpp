#include "coordinate.h"
#include "coordinate_config.h"
#include "../Config.h"
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

CoordinateConfig::CoordinateConfig() 
: simparm::Object("ByCoordinate", "Vary hue with coordinate value"),
  choice("HueCoordinate", "Coordinate to vary hue with", output::binning::FieldChoice::ScaledToUnitInterval)
{
    push_back( choice );
}

CoordinateConfig::CoordinateConfig(const CoordinateConfig& o) 
: ColourScheme(o), simparm::Object(o), choice(o.choice) 
{
    push_back( choice );
}

std::auto_ptr<Backend> CoordinateConfig::make_backend( Config& config, Status& status ) const
{
    return Backend::create< Coordinate >(Coordinate(config.invert(), choice.value().make_scaled_binner()), config, status);
}

}

template <>
std::auto_ptr<ColourScheme> ColourScheme::config_for<colour_schemes::Coordinate>()
{
    return std::auto_ptr<ColourScheme>(new colour_schemes::CoordinateConfig());
}

}
}
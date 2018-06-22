#include "display/Manager.h"
#include "display/DataSource.h"
#include "image/Image.hpp"
#include <limits>

namespace dStorm {
namespace display {

void DataSource::notice_user_key_limits(int, bool, std::string) {}

std::vector<KeyChange> 
KeyChange::make_linear_key( std::pair<float,float> range )
{
    std::vector<KeyChange> rv;
    rv.reserve( 256 );
    for (int i = 0; i <= 255; i++)
        rv.push_back( KeyChange(
            i, Color(i),
            range.first + i * (range.second - range.first) / 255.0) );
    return rv;
}

void Change::make_linear_key(Image::PixelPair range) {
    if ( changed_keys.empty() ) changed_keys.push_back( std::vector<KeyChange>() );
    changed_keys[0] = KeyChange::make_linear_key( std::pair<float,float>( range.first, range.second ) );
}

void DataSource::look_up_key_values( const PixelInfo& info, std::vector<float>& targets )
{
    for (std::vector<float>::iterator 
         i = targets.begin(); i != targets.end(); ++i)
    {
        *i = std::numeric_limits<float>::quiet_NaN();
    }
}

SaveRequest::SaveRequest() 
: scale_bar( 1E-6 * boost::units::si::meter ) {}

}

template <>
const int Image< dStorm::Pixel, 3 >::Dim;

}


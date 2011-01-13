#include "coordinate.h"
#include <dStorm/output/binning/binning.h>
#include <dStorm/output/ResultRepeater.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

dStorm::Display::KeyDeclaration Coordinate::create_key_declaration( int index ) const {
    if ( index != 1 ) throw std::logic_error("Request to create unknown key");

    input::ImageResolution resolution = variable->resolution();
    dStorm::Display::KeyDeclaration rv(
        resolution.unit_symbol,
        resolution.unit_name,
        key_resolution);
    if ( repeater ) {
        rv.can_set_lower_limit = rv.can_set_upper_limit = true;
        std::stringstream s[2];
        s[0] << variable->get_minmax().first;
        s[1] << variable->get_minmax().second;
        rv.lower_limit = s[0].str(); rv.upper_limit = s[1].str();
    }
    return rv;
}

void Coordinate::create_full_key( dStorm::Display::Change::Keys::value_type& into, int index ) const
{
    if ( index != 1 ) {
        BaseType::create_full_key( into, index );
        return;
    }

    const float max_saturation = 1;
    const BrightnessType max_brightness 
        = std::numeric_limits<BrightnessType>::max();
    const int key_count = key_resolution;
    into.reserve( key_count );
    for (int i = 0; i < key_count; ++i) {
        float hue = (i * 0.666f / key_count);
        RGBWeight weights;
        rgb_weights_from_hue_saturation
            ( hue, max_saturation, weights );

        /* Key value in frames */
        float value = variable->map_from_unit_interval( (1.0f * i + 0.5f) / key_count );

        into.push_back( dStorm::Display::KeyChange(
            /* index */ i,
            /* color */ weights * max_brightness,
            /* value */ value ) );
    }
} 


void Coordinate::announce(const output::Output::Announcement& a)
{
    repeater = a.result_repeater;
    variable->announce(a);
}

void Coordinate::announce(const output::Output::EngineResult& er)
{
    if ( is_for_image_number && er.number > 0 )
        set_tone( variable->bin_point(er.first[0]) );
}

void Coordinate::announce(const Localization& l)
{
    if ( ! is_for_image_number )
        set_tone( variable->bin_point(l) );
}

void Coordinate::notice_user_key_limits(int index, bool lower_limit, std::string s)
{
    if ( index == 1 ) {
        assert( repeater );
        if ( ! repeater ) throw std::runtime_error("Missing old localization data for re-keying");
        variable->set_user_limit( lower_limit, s );
        repeater->repeat_results();
    } else
        BaseType::notice_user_key_limits( index, lower_limit, s );
}

}
}
}

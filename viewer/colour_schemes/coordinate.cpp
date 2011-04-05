#include "coordinate.h"
#include <dStorm/output/binning/binning.h>
#include <dStorm/Engine.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

Coordinate::Coordinate( bool invert, std::auto_ptr< output::binning::UserScaled > scaled )
: BaseType(invert), HueSaturationMixer(0,0), variable( scaled ), repeater(NULL),
  is_for_image_number( variable->field_number() == dStorm::Localization::Fields::ImageNumber ) 
{
    currently_mapping = variable->is_bounded();
    set_base_tone( 0, (currently_mapping) ? 1 : 0 );
}

Coordinate::Coordinate( const Coordinate& o )
: BaseType(o), HueSaturationMixer(o), variable( o.variable->clone() ), repeater(o.repeater),
  is_for_image_number(o.is_for_image_number), currently_mapping(o.currently_mapping)
{
}

dStorm::Display::KeyDeclaration Coordinate::create_key_declaration( int index ) const {
    if ( index != 1 ) throw std::logic_error("Request to create unknown key");

    dStorm::Display::KeyDeclaration rv = variable->key_declaration();
    rv.size = key_resolution;
    if ( ! repeater ) {
        rv.can_set_lower_limit = rv.can_set_upper_limit = false;
    }
    return rv;
}

void Coordinate::create_full_key( dStorm::Display::Change::Keys::value_type& into, int index ) const
{
    if ( index != 1 ) {
        BaseType::create_full_key( into, index );
        return;
    }

    if (currently_mapping) {
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
            float value = variable->reverse_mapping( (1.0f * i + 0.5f) / key_count );

            into.push_back( dStorm::Display::KeyChange(
                /* index */ i,
                /* color */ weights * max_brightness,
                /* value */ value ) );
        }
    }
} 


void Coordinate::announce(const output::Output::Announcement& a)
{
    repeater = a.engine;
    variable->announce(a);
    currently_mapping = variable->is_bounded();
    set_base_tone( 0, (currently_mapping) ? 1 : 0 );
}

void Coordinate::announce(const output::Output::EngineResult& er)
{
    if ( currently_mapping && is_for_image_number && er.number > 0 )
        set_tone( variable->bin_point(er.first[0]) * 0.666f );
}

void Coordinate::announce(const Localization& l)
{
    if ( currently_mapping && ! is_for_image_number ) {
        set_tone( variable->bin_point(l) * 0.666f );
    }
}

void Coordinate::notice_user_key_limits(int index, bool lower_limit, std::string s)
{
    if ( index == 1 ) {
        assert( repeater );
        if ( ! repeater ) throw std::runtime_error("Missing old localization data for re-keying");
        variable->set_user_limit( lower_limit, s );
        currently_mapping = variable->is_bounded();
        set_base_tone( 0, (currently_mapping) ? 1 : 0 );
        repeater->repeat_results();
    } else
        BaseType::notice_user_key_limits( index, lower_limit, s );
}

}
}
}

#include "ColourDisplay_impl.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace boost::units;

namespace dStorm {
namespace viewer {
namespace ColourSchemes {

enum ColourPart { V, P, Q, T };
static const ColourPart color_index_table[6][3] 
    = { { V, T, P}, { Q, V, P}, { P, V, T}, { P, Q, V},
        { T, P, V}, { V, P, Q} };

void rgb_weights_from_hue_saturation
    (float hue, float saturation, RGBWeight &array) 
{
    int hue_index = std::max(0, std::min<int>( floor( hue * 6 ), 5 ));
    float f = hue * 6 - hue_index;

    float parts[4];
    parts[V] = 1;
    parts[P] = 1 - saturation;
    parts[Q] = 1 - f*saturation;
    parts[T] = 1 - (1 - f) * saturation;

    for (int c = 0; c < 3; c++)
        array[c] = parts[ color_index_table[hue_index][c] ];

}

static const Eigen::Rotation2D<float> 
    rot_90_deg_clockw( - M_PI / 2 ),
    rot_30_deg_clockw( - M_PI / 6 );

void convert_xy_tone_to_hue_sat( 
    float x, float y, float& hue, float& sat ) 
{
    Eigen::Vector2f v(x,y), ov = v;
    if ( v.squaredNorm() < 1E-4 )  {
        hue = 0;
        sat = 0;
    } else {
        sat = v.norm();

        int rotations = 0;
        if ( v.y() < 0 ) { v *= -1; rotations += 6; }
        if ( v.x() < 0 ) {
            std::swap( v.x(), v.y() );
            v.y() *= -1;
            rotations += 3; 
        }

        while ( v.y() > 0.6 * v.x() ) {
            v = rot_30_deg_clockw * v;
            rotations+= 1;
        }

        float approximate_position = (2*v.y()) / v.x();
        hue = (rotations + approximate_position) / 12;
    }
}

}

template <>
void HueingColorizer<ColourSchemes::TimeHue>
::announce(const output::Output::Announcement& a)
{
    repeater = a.result_repeater;
    origrange[0] = a.first_frame;
    if ( a.last_frame.is_set() )
        origrange[1] = *a.last_frame;
    else
        throw std::runtime_error("Total length of acquisition must be "
                                    "known for colour coding by time.");
    variable.speed = a.speed;
    set_range();
}

template <int Hueing>
void HueingColorizer<Hueing>::set_user_limit(int lower, simparm::optional<Qty> value)
{
    userrange[lower] = value;
    set_range();
}

template <>
void HueingColorizer<ColourSchemes::TimeHue>
    ::notice_user_key_limits(int index, bool lower_limit, std::string s)
{
    if ( index == 1 ) {
        assert( repeater );
        if ( ! repeater ) throw std::runtime_error("Missing old localization data for re-keying");
        Qty v;
        if ( variable.speed.is_set() ) 
            v = atof(s.c_str()) * si::seconds * *variable.speed;
        else
            v = Qty::from_value( atof(s.c_str()) );
        set_user_limit( (lower_limit) ? 0 : 1, v );
        repeater->repeat_results();
    } else
        BaseType::notice_user_key_limits( index, lower_limit, s );
}

template <>
void HueingColorizer<ColourSchemes::ZHue>
    ::notice_user_key_limits(int index, bool lower_limit, std::string s)
{
    if ( index == 1 ) {
        assert( repeater );
        if ( ! repeater ) throw std::runtime_error("Missing old localization data for re-keying");
        set_user_limit( (lower_limit) ? 0 : 1, Qty::from_value( atof(s.c_str()) ) );
        repeater->repeat_results();
    } else
        BaseType::notice_user_key_limits( index, lower_limit, s );
}

template <int Hue>
void HueingColorizer<Hue>::create_full_key( dStorm::Display::Change::Keys::value_type& into, int index ) const
{
    if ( index == 1 ) {
        const float max_saturation = 1;
        const BrightnessType max_brightness 
            = std::numeric_limits<BrightnessType>::max();
        const int key_count = key_resolution;
        into.reserve( key_count );
        for (int i = 0; i < key_count; ++i) {
            float hue = (i * 0.666f / key_count);
            RGBWeight weights;
            ColourSchemes::rgb_weights_from_hue_saturation
                ( hue, max_saturation, weights );

            /* Key value in frames */
            quantity<Unit,float>
                frame = ((range[1] - range[0]) * ((1.0f * i) / key_count) + range[0]);
            float value = key_value(frame);

            into.push_back( dStorm::Display::KeyChange(
                /* index */ i,
                /* color */ weights * max_brightness,
                /* value */ value ) );
        }
    } else
        BaseType::create_full_key( into, index );
} 

template <>
void HueingColorizer<ColourSchemes::ZHue>
::announce(const output::Output::Announcement& a) 
{
    repeater = a.result_repeater;
    assert( a.z_range.is_set() );
    if ( ! a.z_range.is_set() )
        throw std::runtime_error("Maximum Z range must be "
                                    "known for colour coding by Z coordinate.");

    origrange[0] =  a.z_range->lower();
    origrange[1] = a.z_range->upper();
    set_range();
}

template class HueingColorizer<ColourSchemes::TimeHue>;
template class HueingColorizer<ColourSchemes::ZHue>;

}
}

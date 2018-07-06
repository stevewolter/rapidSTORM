#include "viewer/colour_schemes/base.h"
#include "viewer/colour_schemes/HueSaturationMixer.h"
#include "helpers/clone_ptr.hpp"
#include "binning/config.h"
#include "binning/binning.h"
#include "base/Engine.h"
#include "viewer/ColourScheme.h"
#include "viewer/ColourSchemeFactory.h"
#include "simparm/Object.h"
#include "simparm/Entry.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Coordinate : public ColourScheme { 
    HueSaturationMixer mixer;
    std::auto_ptr< binning::UserScaled > variable;
    static const int key_resolution = 100;

    Engine *repeater;
    bool currently_mapping;
    const float range;

    void set_tone( const Localization& l );
    virtual Coordinate* clone_() const { return new Coordinate(*this); }

  public:
    Coordinate( bool invert, std::auto_ptr< binning::UserScaled > scaled, float range );
    Coordinate( const Coordinate& o );
    int key_count() const { return 2; }

    void setSize( const MetaInfo& traits ) {
        ColourScheme::setSize(traits);
        mixer.setSize(traits.size);
    }
    Pixel getPixel(Im::Position p, BrightnessType val) const
        { if ( ! currently_mapping ) return inv( val ); else return inv( mixer.getPixel(p,val) ); }
    Pixel getKeyPixel( BrightnessType val ) const 
        { return inv( mixer.getKeyPixel(val) ); }
    void updatePixel(const Im::Position& p, float oldVal, float newVal) 
        { if ( currently_mapping) mixer.updatePixel(p, oldVal, newVal); }

    void announce(const output::Output::Announcement& a); 
    void announce(const output::Output::EngineResult& er);
    void announce(const Localization&);

    dStorm::display::KeyDeclaration create_key_declaration( int index ) const;
    void create_full_key( dStorm::display::Change::Keys::value_type& into, int index ) const;
    void notice_user_key_limits(int, bool, std::string);
};


Coordinate::Coordinate( bool invert, std::auto_ptr< binning::UserScaled > scaled, float range )
: ColourScheme(invert), mixer(0,0), variable( scaled ), repeater(NULL),
  range(range)
{
    currently_mapping = variable->is_bounded();
    mixer.set_base_tone( 0, (currently_mapping) ? 1 : 0 );
    variable->set_clipping( false );
}

Coordinate::Coordinate( const Coordinate& o )
: ColourScheme(o), mixer(o.mixer), variable( o.variable->clone() ), repeater(o.repeater),
  currently_mapping(o.currently_mapping),
  range(o.range)
{
}

display::KeyDeclaration Coordinate::create_key_declaration( int index ) const {
    if ( index != 1 ) throw std::logic_error("Request to create unknown key");

    display::KeyDeclaration rv = variable->key_declaration();
    rv.size = key_resolution;
    if ( ! repeater ) {
        rv.can_set_lower_limit = rv.can_set_upper_limit = false;
    }
    return rv;
}

void Coordinate::create_full_key( display::Change::Keys::value_type& into, int index ) const
{
    if ( index != 1 ) {
        ColourScheme::create_full_key( into, index );
        return;
    }

    if (currently_mapping) {
        const float max_saturation = 1;
        const uint8_t max_brightness = 255;
        const int key_count = key_resolution;
        into.reserve( key_count );
        for (int i = 0; i < key_count; ++i) {
            float hue = (i * range / key_count);
            RGBWeight weights;
            rgb_weights_from_hue_saturation
                ( hue, max_saturation, weights );

            /* Key value in frames */
            float value = variable->reverse_mapping( (1.0f * i + 0.5f) / key_count );
            display::Color color = weights * max_brightness;

            into.push_back( display::KeyChange(i, color, value) );
        }
    }
} 


void Coordinate::announce(const output::Output::Announcement& a)
{
    repeater = a.engine;
    variable->announce(a);
    currently_mapping = variable->is_bounded();
    mixer.set_base_tone( 0, (currently_mapping) ? 1 : 0 );
}

void Coordinate::announce(const output::Output::EngineResult& er) {
}

void Coordinate::announce(const Localization& l)
{
    if ( currently_mapping ) {
        set_tone( l );
    }
}

void Coordinate::set_tone( const Localization& l ) {
    boost::optional<float> v = variable->bin_point(l);
    if ( v.is_initialized() )
        mixer.set_tone( *v * range );
    else
        mixer.set_tone( 0 );
}

void Coordinate::notice_user_key_limits(int index, bool lower_limit, std::string s)
{
    if ( index == 1 ) {
        assert( repeater );
        if ( ! repeater ) throw std::runtime_error("Missing old localization data for re-keying");
        variable->set_user_limit( lower_limit, s );
        currently_mapping = variable->is_bounded();
        mixer.set_base_tone( 0, (currently_mapping) ? 1 : 0 );
        repeater->repeat_results();
    } else
        ColourScheme::notice_user_key_limits( index, lower_limit, s );
}

struct CoordinateConfig : public ColourSchemeFactory
{
    binning::FieldChoice choice;
    simparm::Entry<double> range;
    simparm::BaseAttribute::ConnectionStore listening;
    default_on_copy< boost::signals2::signal<void()> > change;

    CoordinateConfig();
    CoordinateConfig(const CoordinateConfig&);
    CoordinateConfig* clone() const { return new CoordinateConfig(*this); }
    std::auto_ptr<ColourScheme> make_backend( bool invert ) const;
    void add_listener( simparm::BaseAttribute::Listener );
    void attach_ui( simparm::NodeHandle );
};

CoordinateConfig::CoordinateConfig() 
: ColourSchemeFactory("ByCoordinate", "Vary hue with coordinate value"),
  choice("HueCoordinate", "Coordinate to vary hue with", binning::InteractivelyScaledToInterval, "Hue"),
  range("HueRange", "Range of hue", 0.666)
{
}

CoordinateConfig::CoordinateConfig(const CoordinateConfig& o) 
: ColourSchemeFactory(o), choice(o.choice), range(o.range)
{
}

void CoordinateConfig::add_listener( simparm::BaseAttribute::Listener l ) {
    choice.add_listener( l );
    change.connect( l );
}

void CoordinateConfig::attach_ui( simparm::NodeHandle at ) {
    listening = range.value.notify_on_value_change( change );
    simparm::NodeHandle r = attach_parent(at);
    choice.attach_ui( r );
    range.attach_ui( r );
}

std::auto_ptr<ColourScheme> CoordinateConfig::make_backend(bool invert) const
{
    return std::auto_ptr<ColourScheme>( new Coordinate(invert, choice().make_user_scaled_binner(), range()) );
}

std::auto_ptr<ColourSchemeFactory> make_coordinate_factory()
{
    return std::auto_ptr<ColourSchemeFactory>(new colour_schemes::CoordinateConfig());
}


}
}
}

#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_Coordinate_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_Coordinate_H

#include "base.h"
#include "HueSaturationMixer.h"
#include <dStorm/helpers/clone_ptr.hpp>
#include <dStorm/output/binning/binning_decl.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Coordinate : public Base { 
    HueSaturationMixer mixer;
    std::auto_ptr< output::binning::UserScaled > variable;
    static const int key_resolution = 100;

    Engine *repeater;
    bool is_for_image_number, currently_mapping;
    const float range;

    void set_tone( const Localization& l );
    virtual Coordinate* clone_() const { return new Coordinate(*this); }

  public:
    Coordinate( bool invert, std::auto_ptr< output::binning::UserScaled > scaled, float range );
    Coordinate( const Coordinate& o );
    int key_count() const { return 2; }

    void setSize( const MetaInfo& traits ) {
        Base::setSize(traits);
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


}
}
}

#endif

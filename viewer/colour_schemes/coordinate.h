#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_Coordinate_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_Coordinate_H

#include "base.h"
#include "HueSaturationMixer.h"
#include <dStorm/helpers/clone_ptr.hpp>
#include <dStorm/output/binning/binning_decl.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Coordinate : public Base<unsigned char>, public HueSaturationMixer { 
  public:
    typedef Base<unsigned char> BaseType;
    typedef BaseType::BrightnessType BrightnessType;

    static const int KeyCount = 2;

  private:
    std::auto_ptr< output::binning::UserScaled > variable;
    static const int key_resolution = 100;

    Engine *repeater;
    bool is_for_image_number, currently_mapping;
    const float range;

  public:
    Coordinate( bool invert, std::auto_ptr< output::binning::UserScaled > scaled, float range );
    Coordinate( const Coordinate& o );

    void setSize( const input::Traits<BinnedImage>& traits ) {
        BaseType::setSize(traits);
        HueSaturationMixer::setSize(traits.size);
    }
    Pixel getPixel(const Im::Position& p, BrightnessType val)
        { if ( ! currently_mapping ) return inv( val ); else return inv( HueSaturationMixer::getPixel(p,val) ); }
    Pixel getKeyPixel( unsigned char val ) const 
        { return inv( HueSaturationMixer::getKeyPixel(val) ); }
    void updatePixel(const Im::Position& p, float oldVal, float newVal) 
        { if ( currently_mapping) HueSaturationMixer::updatePixel(p, oldVal, newVal); }

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

#ifndef DSTORM_ENIGNE_INPUTTRAITS_H
#define DSTORM_ENIGNE_INPUTTRAITS_H

#include "engine/InputPlane.h"
#include "input/Traits.h"
#include "engine/Image_decl.h"
#include "localization/Traits.h"
#include "boost/units/systems/camera/frame_rate.hpp"

namespace dStorm {
namespace input {

template <>
class Traits< engine::ImageStack > 
: public input::BaseTraits
{
public:
    int plane_count() const { return planes_.size(); }
    engine::InputPlane &plane( int i ) { return planes_[i]; }
    const engine::InputPlane &plane( int i ) const { return planes_[i]; }

    image::MetaInfo<2>& image( int i ) { return planes_[i].image; }
    const image::MetaInfo<2>& image( int i ) const { return planes_[i].image; }
    traits::Optics& optics( int i ) { return planes_[i].optics; }
    const traits::Optics& optics( int i ) const { return planes_[i].optics; }
    const traits::Projection& projection( int i ) const { return planes_[i].projection(); }

    typedef engine::InputPlane value_type;
    typedef std::vector< engine::InputPlane >::iterator iterator;
    typedef std::vector< engine::InputPlane >::const_iterator const_iterator;
    typedef std::vector< engine::InputPlane >::reference reference;
    typedef std::vector< engine::InputPlane >::const_reference const_reference;

    iterator begin() { return planes_.begin(); }
    const_iterator begin() const { return planes_.begin(); }
    iterator end() { return planes_.end(); }
    const_iterator end() const { return planes_.end(); }

    std::pair<samplepos,samplepos> size_in_sample_space() const;

    localization::MetaInfo<localization::ImageNumber::ValueType>& image_number() { return in; }
    const localization::MetaInfo<localization::ImageNumber::ValueType>& image_number() const { return in; }

    boost::units::quantity<boost::units::camera::frame_rate>
        frame_rate;
    int fluorophore_count;

    std::string desc() const { return "image"; }
    Traits* clone() const { return new Traits(*this); }

    void push_back( const image::MetaInfo<2>&, const traits::Optics& );
    void push_back( const engine::InputPlane& );
    void clear();

    Traits();
    Traits( const image::MetaInfo<2>& );

    std::ostream& print_psf_info( std::ostream& ) const;

private:
    std::vector< engine::InputPlane > planes_;
    localization::MetaInfo<localization::ImageNumber::ValueType> in;
};

}
}

#endif

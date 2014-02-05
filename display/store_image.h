#ifndef DSTORM_DISPLAY_STORE_IMAGE_H
#define DSTORM_DISPLAY_STORE_IMAGE_H

#include <string>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>

namespace dStorm {
namespace display {

class Change;

struct StorableImage {
    const Change& image;
    std::string filename;
    boost::units::quantity< boost::units::si::length > scale_bar;

    StorableImage( const std::string& filename, const Change& image );
};

/** Store the given image on the hard disk. The Change structure given should have the resize and clear flags set. */
void store_image( std::string filename, const Change& image );
void store_image( const StorableImage& );

}
}

#endif

#ifndef DSTORM_TIFF_OPEN_FILE_H
#define DSTORM_TIFF_OPEN_FILE_H

#include <dStorm/input/Traits.h>
#include <simparm/Node.hh>
#include <simparm/Entry.hh>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIFFIO_H
#include <tiffio.h>
#endif

#include <dStorm/image/MetaInfo.h>
#include <dStorm/image/Image.h>
#include <dStorm/engine/InputTraits.h>

namespace dStorm {
class TIFFOperation;
namespace tiff {


struct Config : public simparm::Object {
    simparm::BoolEntry ignore_warnings, determine_length;

    Config();
    void registerNamedEntries() {
        push_back( ignore_warnings ); 
        push_back( determine_length ); 
    }
};

class OpenFile : boost::noncopyable {
    typedef uint16_t Pixel;
    static const int Dim = 3;
public:
    typedef dStorm::Image< Pixel, Dim > Image;
private:
#ifdef HAVE_TIFFIO_H
    ::TIFF *tiff;
#endif
    bool ignore_warnings, determine_length;
    std::string file_ident;

    int current_directory;

    dStorm::ImageTypes<Dim>::Size size;
    int _no_images;
    image::MetaInfo<2>::Resolutions resolution;

    template <typename PixelType>
    void read_data_( Image&, TIFFOperation& ) const;

public:
    OpenFile(const std::string& filename, const Config&, simparm::Node&);
    ~OpenFile();

    std::auto_ptr< input::Traits<engine::ImageStack> > 
        getTraits( bool final, simparm::Entry<long>& );
    std::auto_ptr< input::BaseTraits > getTraits();

    void seek_to_image( simparm::Node& msg, int image );
    bool next_image( simparm::Node& msg );
    Image read_image( simparm::Node& msg ) const;
};

}
}

#endif

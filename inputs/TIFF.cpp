#define DSTORM_TIFFLOADER_CPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIFFIO_H

#include "debug.h"

#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <stdint.h>

#include <simparm/ChoiceEntry_Impl.hh>

#include <dStorm/Image.h>
#include <tiffio.h>

#include "TIFF.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/ImageTraits.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/units/base_units/us/inch.hpp>

#include "TIFFOperation.h"
#include <dStorm/input/FileContext.h>

using namespace std;

namespace dStorm {
namespace TIFF {

template<typename Pixel>
Source<Pixel>::Source( boost::shared_ptr<OpenFile> file )
: simparm::Set("TIFF", "TIFF image reader"),
  BaseSource( static_cast<simparm::Node&>(*this),    
      Flags() ),
  file(file)
{
}

template<typename Pixel>
class Source<Pixel>::iterator 
: public boost::iterator_facade<iterator,Image,std::random_access_iterator_tag>
{
    mutable OpenFile* src;
    mutable simparm::Node* msg;
    int directory;
    mutable Image img;

    void go_to_position() const {
        TIFFOperation op( "in reading TIFF file",
                          *msg, src->ignore_warnings );
        int rv = TIFFSetDirectory(src->tiff, directory);
        if ( rv == 1 ) {
            src->current_directory = directory;
        } else {
            op.throw_exception_for_errors();
        }
    }
    void check_position() const {
        if ( src->current_directory != directory )
            go_to_position();
    }
    void check_params() const;

  public:
    iterator() : src(NULL), msg(NULL) {}
    iterator(Source &s) : src(s.file.get()), msg(&s), directory(0) {}

    Image& dereference() const; 
    bool equal(const iterator& i) const {
        DEBUG( "Comparing " << src << " " << i.src << " " << directory << " " << i.directory );
        return (src == i.src) && (src == NULL || directory == i.directory);
    }
    void increment() { 
        TIFFOperation op( "in reading TIFF file",
                          *msg, src->ignore_warnings );
        check_position();
        img.invalidate(); 
        if ( TIFFReadDirectory(src->tiff) != 1 ) {
            op.throw_exception_for_errors();
            /* Code from here only executed when no error
             * was encountered */
            DEBUG( "Setting iterator to NULL" );
            src = NULL;
        } else {
            directory++; 
            src->current_directory = directory;
        }
    }
    void decrement() { 
        img.invalidate(); 
        if ( directory == 0 ) 
            src = NULL; 
        else {
            --directory;
            go_to_position();
        }
    }
    void advance(int n) { 
        if (n) {
            img.invalidate(); 
            directory += n;
            go_to_position();
        }
    }
    int distance_to(const iterator& i) {
        return i.directory - directory;
    }
};

template<typename Pixel>
void
Source<Pixel>::iterator::check_params() const
{
    ::TIFF *tiff = src->tiff;
    uint32_t width, height;
    uint16_t bitspersample;
    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
    TIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &bitspersample );

    if ( int(width) != src->_width || int(height) != src->_height ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff)
            << " has dimensions (" << width << ", " << height
            << ") different from first image (" << src->_width
            << ", " << src->_height << "). Aborting.";
        throw std::runtime_error( error.str() );
    }
    if ( bitspersample != sizeof(Pixel)*8 ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff) << " has "
                << bitspersample << " bits per pixel, but "
                << sizeof(Pixel)*8 << " bits are necessary.";
        throw std::runtime_error( error.str() );
    }
}

template<typename Pixel>
typename Source<Pixel>::Image&
Source<Pixel>::iterator::dereference() const
{ 
    if ( img.is_invalid() ) {
        TIFFOperation op( "in reading TIFF file",
                          *msg, src->ignore_warnings );
        check_position();
        check_params();

        typename Image::Size sz;
        ::TIFF *tiff = src->tiff;
        tsize_t strip_size = TIFFStripSize( tiff );
        tstrip_t strip_count = TIFFNumberOfStrips( tiff );
        sz.x() = src->_width * cs_units::camera::pixel;
        sz.y() = src->_height * cs_units::camera::pixel;
        Image i( sz, directory * cs_units::camera::frame);

        DEBUG("Reading image " << directory << " " << i.size());
        assert( i.size() >= (strip_size * strip_count / sizeof(Pixel)) * cs_units::camera::pixel * cs_units::camera::pixel );

        for (tstrip_t strip = 0; strip < strip_count; strip++) {
            TIFFReadEncodedStrip( tiff, strip, 
                i.ptr() + (strip * strip_size / sizeof(Pixel)),
                strip_size );
        }
        img = i;

        op.throw_exception_for_errors();
    }

    return img;
}

template<typename Pixel>
typename Source<Pixel>::base_iterator
Source<Pixel>::begin() {
    return base_iterator( iterator(*this) );
}

template<typename Pixel>
typename Source<Pixel>::base_iterator
Source<Pixel>::end() {
    return base_iterator( iterator() );
}

Config::Config()
: simparm::Object("TIFF", "TIFF file"),
  ignore_warnings("IgnoreLibtiffWarnings",
    "Ignore libtiff warnings", false),
  determine_length("DetermineFileLength",
    "Determine length of file", false)
{
    ignore_warnings.userLevel 
        = simparm::Object::Intermediate;
}

template<typename Pixel>
ChainLink<Pixel>::ChainLink() 
{
}

template<typename Pixel>
Source< Pixel >*
ChainLink<Pixel>::makeSource()
{
    Source<Pixel>* ptr =
        new Source<Pixel>( file );
    return ptr;
}

template<typename Pixel>
void ChainLink<Pixel>::context_changed( ContextRef ocontext )
{
    FileContext& context = dynamic_cast<FileContext&>(*ocontext);

    if ( ! file.get() || file->for_file() != context.input_file ) {
        file.reset();

        try {
            boost::shared_ptr<FileMetaInfo> info;
            file.reset( new OpenFile( context.input_file, config, config ) );

            info->traits = file->getTraits<Pixel>();
            info->accepted_basenames.push_back( make_pair("extension_tif", ".tif") );
            info->accepted_basenames.push_back( make_pair("extension_tiff", ".tiff") );

            this->notify_of_trait_change( info );
        } catch(...) {
            this->notify_of_trait_change( TraitsRef() );
        }
    }
}

template<typename Pixel>
typename Source<Pixel>::TraitsPtr 
Source<Pixel>::get_traits() {
    return file->getTraits<Pixel>();
}

template<typename Pixel>
Source<Pixel>::~Source() {}

//template class ChainLink<unsigned char>;
template class ChainLink<unsigned short>;
//template class ChainLink<unsigned int>;
//template class ChainLink<float>;
//template class ChainLink<double>;

}
}

#endif

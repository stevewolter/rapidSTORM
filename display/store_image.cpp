#include "debug.h"
#include "display/store_image.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <cmath>
#include <boost/ptr_container/ptr_list.hpp>
#include "image/slice.h"
#include "simparm/Message.h"
#include <stdio.h>
#include <string>
#include "Pixel.h"
#include <vector>
#include "display/DataSource.h"

#ifdef USE_GRAPHICSMAGICK
#include <Magick++.h>
#endif

static const char *SI_prefixes[]
= { "f", "p", "n", "u", "m", "", "k", "M", "G", "T",
    "E" };
std::string SIize( float value ) {
    if ( value < 1E-21 ) return "0";
    int prefix = int(floor(log10(value))) / 3;
    prefix = std::max(-5, std::min(prefix, 5));
    float rv = value / pow(1000, prefix);
    char buffer[128];
    snprintf( buffer, 128, "%.3g %s", rv, 
              SI_prefixes[prefix + 5] );
    return buffer;
}

namespace dStorm {
namespace display {

#ifdef USE_GRAPHICSMAGICK
template <int MagickDepth>
inline void 
    make_magick_pixel( Magick::PixelPacket& mp, const dStorm::Pixel& p );

template <>
inline void 
    make_magick_pixel<8>( Magick::PixelPacket& mp,
                          const dStorm::Pixel& p )
{
    mp.red = p.red();
    mp.blue = p.blue();
    mp.green = p.green();
    mp.opacity = p.alpha();
}

template <>
inline void 
    make_magick_pixel<16>( Magick::PixelPacket& mp,
                           const dStorm::Pixel& p )
{
    mp.red = (p.red() | (p.red() << 8));
    mp.blue = (p.blue() | (p.blue() << 8));
    mp.green = (p.green() | (p.green() << 8));
    mp.opacity = (p.alpha() | (p.alpha() << 8));
}

static
std::auto_ptr<Magick::Image> 
make_palette( const std::vector<KeyChange>& key ) 
{
    DEBUG("Making palette");
    std::auto_ptr< Magick::Image > rv
        ( new Magick::Image(
            Magick::Geometry( key.size(), 1 ),
            Magick::ColorRGB( 0, 0, 0 ) ) );
    rv->type(Magick::TrueColorType);

    Magick::PixelPacket *pixels = 
        rv->getPixels(0, 0, key.size(), 1);
    for (size_t i = 0; i < key.size(); i++)
        make_magick_pixel<QuantumDepth>
            ( pixels[i], key[i].color );

    rv->syncPixels();
    DEBUG("Made palette");
    return rv;
}

static std::auto_ptr<Magick::Image>
make_unannotated_key_image( 
    int width, int color_bar_height, int annotation_area_height,
    Magick::Color background,
    std::auto_ptr<Magick::Image> palette
) {
    DEBUG("Making unannotated key");
    int border = 5;
    int w = std::max(1, width-2*border);
    Magick::Geometry key_img_size 
        = Magick::Geometry(w, color_bar_height);
    key_img_size.aspect( true );
    palette->sample( key_img_size );

    std::auto_ptr<Magick::Image> rv ( 
        new Magick::Image(
            Magick::Geometry(width, color_bar_height+annotation_area_height),
            background ) );
    rv->composite( *palette, (width-w)/2, 0, Magick::OverCompositeOp );
    DEBUG("Made unannotated key");
    return rv;
}

static
std::auto_ptr<Magick::Image> 
make_key_image( 
    int width,
    Magick::Color foreground,
    Magick::Color background,
    const dStorm::display::KeyDeclaration& kd,
    const std::vector<KeyChange>& key )
{
    DEBUG("Making annotated key");
    std::auto_ptr<Magick::Image> rv, palette = make_palette(key);
    int lh = palette->fontPointsize();
    int midline = 20;
    int text_area_height = 5*lh;
    int key_annotation_height = lh;
    rv = make_unannotated_key_image( 
        width, midline, text_area_height + key_annotation_height,
        background, palette);

    if ( ! key.empty() ) {
        DEBUG("Annotating");
        Magick::TypeMetric metrics;
        rv->strokeColor( foreground );
        rv->fillColor( foreground );
        for (int i = lh/3; i < width-(lh-lh/6); i += lh )
        {
            int index = round(i * key.size() * 1.0 / width );
            index = std::max(0, std::min(index, int(key.size()) -1));
            DEBUG("Annotating at index " << index << " for key size " << key.size() << " and position " << i << " of " << width);
            float value = key[ index ].value;
            std::string s = SIize(value);
            rv->annotate(s, 
                Magick::Geometry( lh, text_area_height-5,
                                    i-lh/6, midline+5),
                Magick::NorthWestGravity, 90 );
            rv->draw( Magick::DrawableLine( i, midline, i, midline+3 ) );
        }

        DEBUG("Writing key annotation");
        std::string message = "Key: " + kd.description;
        if ( width >= 20*lh )
        rv->annotate( message,
            Magick::Geometry( width, 
                key_annotation_height, 0, midline+text_area_height ),
                Magick::NorthGravity, 0);
    }

    DEBUG("Made annotated key");
    return rv;
}

static void write_main_image(
    Magick::Image& image,
    int width,
    const ImageChange& whole_image,
    const Change::PixelQueue& small_changes,
    int layer
) {
    DEBUG("Writing main image");
    int cols = width, rows = whole_image.new_image.height_in_pixels();
    DEBUG("Using stride " << width);

    dStorm::Image<Pixel,2> slice = whole_image.new_image.slice(2, layer * camera::pixel);
    for (int y = 0; y < rows; y++) {
        Magick::PixelPacket *pixels = image.setPixels
            ( 0, y, cols, 1 );
        dStorm::Image<Pixel,1> row = slice.slice(1, y * camera::pixel);
        dStorm::Image<Pixel,1>::const_iterator p = row.begin();
        for (int x = 0; x < cols; x++)
            make_magick_pixel<QuantumDepth>( pixels[x], *p++ );
        image.syncPixels();
    }
    DEBUG("Wrote main image");
}

static void write_scale_bar(
    Magick::Image& image,
    dStorm::traits::ImageResolution ppm,
    int width,
    int x_offset )
{
    int lh = image.fontPointsize();
    if ( int(image.rows()) < 18+lh ) return;
    int y_offset = image.rows()-12-lh;

    DEBUG("Writing scale bar at " << x_offset << " " << image.rows()-12-lh << " down to " 
        << x_offset+width << " and " << y_offset+5 << " with unit symbol " << ppm.unit_symbol);
    image.draw( Magick::DrawableRectangle( x_offset, y_offset, x_offset+width, y_offset+5 ) );

    DEBUG("Writing scale bar annotation");
    image.annotate(
         SIize(width * camera::pixel * ppm.value) + ppm.unit_symbol, 
            Magick::Geometry(width, lh, x_offset, y_offset+10),
            Magick::CenterGravity );
    DEBUG("Wrote scale bar annotation");
}
#endif

std::auto_ptr<Magick::Image> create_layer( const Change& image, int layer, quantity<si::length> scale_bar ) {
    DEBUG("Image to store has width " << image.resize_image.size.x() << " and height " << image.resize_image.size.y()
        <<" and has " << image.changed_keys.size() << " keys");
    int width = image.resize_image.size.x() / camera::pixel; 
    int main_height = image.resize_image.size.y() / camera::pixel;
    int total_height = main_height;
    int border_after_image = 10;

    Color bg = image.clear_image.background;
    Magick::ColorRGB background 
        ( bg.red()/255.0, bg.green()/255.0, bg.blue()/255.0 ),
                     foreground 
        ( 1.0 - background.red(), 1.0 - background.green(), 
          1.0 - background.blue() );

    boost::ptr_list< Magick::Image > key_imgs;
    if ( image.resize_image.size[0] > 256 * camera::pixel &&
         image.resize_image.size[1] > 256 * camera::pixel ) {
        for ( unsigned int i = 0; i < image.changed_keys.size(); ++i ) {
            key_imgs.push_back( make_key_image( 
                width, foreground, background, 
                image.resize_image.keys[i], image.changed_keys[i] ) );
            total_height += border_after_image + key_imgs.back().rows();
        }
    }

    DEBUG("Creating image sized " << width << " by " << total_height);
    std::auto_ptr<Magick::Image> img( new Magick::Image( Magick::Geometry(width, total_height), background ) );
    img->type(Magick::TrueColorType);
    img->strokeColor( foreground );
    img->fillColor( foreground );

    write_main_image( *img, width, image.image_change, image.change_pixels, layer );
    int key_pos = main_height;
    for (boost::ptr_list< Magick::Image >::iterator i = key_imgs.begin(); i != key_imgs.end(); ++i )
    {
        img->composite( *i, 0, key_pos, Magick::OverCompositeOp );
        key_pos += i->rows();
    }
    int scale_bar_width = std::min( width/3, 100 );
    if ( image.resize_image.pixel_sizes[0].is_in_dpm() ) {
        scale_bar_width = 
            round( (image.resize_image.pixel_sizes[0].in_dpm() * scale_bar) / camera::pixel );
    }
    if ( image.resize_image.pixel_sizes[0].value > 0 / camera::pixel
          && image.resize_image.pixel_sizes[1].value > 0 / camera::pixel ) 
    {
        write_scale_bar( *img, image.resize_image.pixel_sizes[0],
                     scale_bar_width, std::max(0, width-scale_bar_width-5 ) );
        DEBUG("Wrote scale bar");
        if ( image.resize_image.pixel_sizes[0].is_in_dpm() && 
             image.resize_image.pixel_sizes[1].is_in_dpm() )
        {
            img->resolutionUnits( Magick::PixelsPerCentimeterResolution );
            unsigned int pix_per_cm[2];
            for (int i = 0; i < 2; ++i)
                pix_per_cm[i] = int( image.resize_image.pixel_sizes[i].in_dpm() * (0.01 * boost::units::si::metre) / camera::pixel );
            img->density(Magick::Geometry(pix_per_cm[0], pix_per_cm[1]));
        }
    }
    
    return img;
}

void store_image_impl( const StorableImage& i )
{
    const Change& image = i.image;
    DEBUG("Storing image");
    if ( !image.do_resize || image.resize_image.keys.size() != image.changed_keys.size() )
        throw std::logic_error("Key information not given completely when saving image");
    if ( ! image.do_clear )
        throw std::logic_error("No background color defined for image");

#if !defined(USE_GRAPHICSMAGICK) 
    throw std::runtime_error("Cannot save images: Magick library not used in compilation");
#else
    std::vector< Magick::Image > layers;
    for (int z = 0; z < image.resize_image.size.z().value(); ++z)
        layers.push_back( *create_layer( image, z, i.scale_bar ) );
    try {
        for ( std::vector<Magick::Image>::iterator im = layers.begin(); im != layers.end(); ++im )
            im->compressType( Magick::RunlengthEncodedCompression );
        writeImages( layers.begin(), layers.end(), i.filename );
    } catch ( const Magick::Error& e ) {
        throw std::runtime_error(e.what());
    }

#endif
}

void store_image( std::string filename, const Change& image )
{
    store_image_impl( StorableImage( filename, image ) );
}
void store_image( const StorableImage& i ) {
    store_image_impl(i);
}

StorableImage::StorableImage( const std::string& filename, const Change& image )
: image(image), filename(filename), scale_bar( 1E-6 * boost::units::si::meter )
{
}

}
}

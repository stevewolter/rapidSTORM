#include "debug.h"
#include "wxManager.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_MAGICK___H
#include <Magick++.h>
#endif
#include <cmath>
#include <boost/ptr_container/ptr_list.hpp>
#include <dStorm/ImageTraits.h>
#include <simparm/Message.hh>

static const char *SI_prefixes[]
= { "f", "p", "n", "µ", "m", "", "k", "M", "G", "T",
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
namespace Display {

#ifdef HAVE_LIBGRAPHICSMAGICK__
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
make_palette( const data_cpp::Vector<KeyChange>& key ) 
{
    DEBUG("Making palette");
    std::auto_ptr< Magick::Image > rv
        ( new Magick::Image(
            Magick::Geometry( key.size(), 1 ),
            Magick::ColorRGB( 0, 0, 0 ) ) );
    rv->type(Magick::TrueColorType);

    Magick::PixelPacket *pixels = 
        rv->getPixels(0, 0, key.size(), 1);
    for (int i = 0; i < key.size(); i++)
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
    const dStorm::Display::KeyDeclaration& kd,
    const data_cpp::Vector<KeyChange>& key )
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

    DEBUG("Annotating");
    Magick::TypeMetric metrics;
    rv->strokeColor( foreground );
    rv->fillColor( foreground );
    for (int i = lh/3; i < width-(lh-lh/6); i += lh )
    {
        int index = round(i * key.size() * 1.0 / width );
        index = std::max(0, std::min(index, key.size()));
        DEBUG("Annotating at index " << index << " for key size " << key.size() << " and position " << i << " of " << width);
        float value = key[ index ].value;
        std::string s = SIize(value);
        rv->annotate(s, 
            Magick::Geometry( lh, text_area_height-5,
                                i-lh/6, midline+5),
            Magick::NorthWestGravity, 90 );
        rv->draw( Magick::DrawableLine( i, midline,
                                        i, midline+3 ) );
    }

    DEBUG("Writing key annotation");
    std::string message = "Key: " + kd.description;
    if ( width >= 20*lh )
      rv->annotate( message,
        Magick::Geometry( width, 
            key_annotation_height, 0, midline+text_area_height ),
            Magick::NorthGravity, 0);

    DEBUG("Made annotated key");
    return rv;
}

static void write_main_image(
    Magick::Image& image,
    int width,
    const ImageChange& whole_image,
    const Change::PixelQueue& small_changes
) {
    DEBUG("Writing main image");
    int cols = width, rows = whole_image.new_image.height_in_pixels();
    DEBUG("Using stride " << width);

    const Pixel *p = whole_image.new_image.ptr();
    for (int y = 0; y < rows; y++) {
        Magick::PixelPacket *pixels = image.setPixels
            ( 0, y, cols, 1 );
        for (int x = 0; x < cols; x++) {
            make_magick_pixel<QuantumDepth>( pixels[x], *p );
            ++p;
        }
        image.syncPixels();
    }
    DEBUG("Wrote main image");
}

static void write_scale_bar(
    Magick::Image& image,
    dStorm::input::ImageResolution ppm,
    int width,
    int x_offset )
{
    int lh = image.fontPointsize();
    if ( int(image.rows()) < 18+lh ) return;
    int y_offset = image.rows()-12-lh;

    DEBUG("Writing scale bar at " << x_offset << " " << image.rows()-12-lh << " down to " 
        << x_offset+width << " and " << y_offset+5 << " with unit symbol " << ppm.unit_symbol);
    image.draw( Magick::DrawableRectangle( 
            x_offset, y_offset, x_offset+width, y_offset+5 ) );

    DEBUG("Writing scale bar annotation");
    image.annotate(
         SIize(width * camera::pixel * ppm.value) + ppm.unit_symbol, 
            Magick::Geometry(width, lh, x_offset, y_offset+10),
            Magick::CenterGravity );
    DEBUG("Wrote scale bar annotation");
}
#endif

void wxManager::store_image(
    std::string filename,
    const Change& image )
{
    DEBUG("Storing image");
    if ( !image.do_resize || !image.do_clear || image.resize_image.keys.size() != image.changed_keys.size() )
        throw std::logic_error("No complete image given for store_image");
#if !defined(HAVE_LIBGRAPHICSMAGICK__) || !defined(HAVE_MAGICK___H)
    throw std::runtime_error("Cannot save images: Magick library not used in compilation");
#else
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
    for ( unsigned int i = 0; i < image.changed_keys.size(); ++i ) {
        key_imgs.push_back( make_key_image( 
            width, foreground, background, 
            image.resize_image.keys[i], image.changed_keys[i] ) );
        total_height += border_after_image + key_imgs.back().rows();
    }

    DEBUG("Creating image sized " << width << " by " << total_height);
    Magick::Image img( Magick::Geometry(width, total_height), background );
    img.type(Magick::TrueColorType);
    img.strokeColor( foreground );
    img.fillColor( foreground );

    write_main_image( img, width, image.image_change, image.change_pixels );
    int key_pos = main_height;
    for (boost::ptr_list< Magick::Image >::iterator i = key_imgs.begin(); i != key_imgs.end(); ++i )
    {
        img.composite( *i, 0, key_pos, Magick::OverCompositeOp );
        key_pos += i->rows();
    }
    int scale_bar_width = std::min( width/3, 100 );
    if ( image.resize_image.pixel_sizes[0].value > 0 / camera::pixel
          && image.resize_image.pixel_sizes[1].value > 0 / camera::pixel ) 
    {
        write_scale_bar( img, image.resize_image.pixel_sizes[0],
                     scale_bar_width, std::max(0, width-scale_bar_width-5 ) );
        DEBUG("Wrote scale bar");
        try {
            img.resolutionUnits( Magick::PixelsPerCentimeterResolution );
            unsigned int pix_per_cm[2];
            for (int i = 0; i < 2; ++i)
                pix_per_cm[i] = int( image.resize_image.pixel_sizes[i].in_dpm() * (0.01 * boost::units::si::metre) / camera::pixel );
            img.density(Magick::Geometry(pix_per_cm[0], pix_per_cm[1]));
        } catch (...) {}
    }
    
    try {
        img.write( filename );
    } catch ( const Magick::Error& e ) {
        throw std::runtime_error(e.what());
    }

#endif
}

}
}

#include "debug.h"
#include "GUI.h"
#include <simparm/Node.h>
#include <dStorm/display/Manager.h>
#include <dStorm/display/display_normalized.hpp>
#include <dStorm/image/crop.h>
#include <dStorm/image/minmax.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/convert.h>
#include <dStorm/image/extend.h>
#include <dStorm/image/normalize.h>
#include <dStorm/image/slice.h>

namespace dStorm {
namespace estimate_psf_form {

struct DisplayHandler 
: public dStorm::display::DataSource
{
    boost::mutex mutex;
    boost::condition_variable is_closed_changed;
    bool is_closed;
    std::auto_ptr< dStorm::display::Change > next_change;
    dStorm::display::Image image;
    boost::ptr_vector<Tile>& tiles;
    int fluorophore_count;
    std::auto_ptr< display::WindowHandle > handle;

    std::auto_ptr<display::Change> get_changes() { 
        std::auto_ptr< dStorm::display::Change > fresh( new dStorm::display::Change(1) );
        boost::lock_guard<boost::mutex> lock(mutex);
        std::swap( next_change, fresh );
        return fresh; 
    }

    void notice_closed_data_window() {
        DEBUG("Noticing closed data window");
        boost::lock_guard<boost::mutex> lock(mutex);
        is_closed = true;
        is_closed_changed.notify_all();
        DEBUG("Noticed closed data window for " << this);
    }

    void notice_drawn_rectangle (int xf, int xt, int yf, int yt); 
    void colour_fluorophore( const Tile& );

  public:
    DisplayHandler( std::auto_ptr< dStorm::display::Change > change, boost::ptr_vector<Tile>& tiles, int fluorophore_count, simparm::NodeHandle ui )
        : is_closed(false), next_change(change), 
          image( next_change->image_change.new_image), tiles(tiles),
          fluorophore_count( fluorophore_count ) 
    {
        DEBUG("Fluorophore count is " << fluorophore_count << " for display handler " << this);
        for ( boost::ptr_vector<Tile>::const_iterator i = this->tiles.begin(); i != this->tiles.end(); ++i ) {
            colour_fluorophore( *i );
        }
        dStorm::display::WindowProperties props;
        props.name = "Select spots for PSF form estimation";
        props.flags.notice_drawn_rectangle();
        props.initial_size = next_change->resize_image;
        handle = ui->get_image_window( props, *this );
    }

    ~DisplayHandler() { DEBUG( "Destructing " << this ); }

    void wait_for_destruction() {
        boost::unique_lock<boost::mutex> lock(mutex);
        while ( ! is_closed )
            is_closed_changed.wait( lock );
    }
};

void DisplayHandler::notice_drawn_rectangle(int xf, int xt, int yf, int yt) {
        DEBUG("Processing notice_drawn_rectangle");
        boost::lock_guard<boost::mutex> lock(mutex);
        Eigen::Array< boost::units::quantity<boost::units::camera::length, int>, 2, 2 > b;
        b(0,0) = xf * camera::pixel;
        b(1,0) = yf * camera::pixel;
        b(0,1) = xt * camera::pixel;
        b(1,1) = yt * camera::pixel;
        for ( boost::ptr_vector<Tile>::iterator i = tiles.begin(); i != tiles.end(); ++i ) {
            /* Check if there might be any overlap between the regions. */
            if ( (b.col(0) > i->region_end.head<2>().array()).any() || (b.col(1) < i->region_start.head<2>().array()).any() )
                continue;
            int overlap_area = 1;
            int region_area = ( i->region_end.x() - i->region_start.x() + 1 * camera::pixel ).value() *  
                              ( i->region_end.y() - i->region_start.y() + 1 * camera::pixel ).value();
            for (int j = 0; j < 2; ++j)
                overlap_area *= (std::min( b(j,1), i->region_end[j] ) 
                               - std::max( b(j,0), i->region_start[j] )).value();

            DEBUG("Region area is " << region_area << " and overlap " << overlap_area );
            if ( overlap_area >= region_area / 2 ) {
                DEBUG("Using region");
                i->fluorophore = (i->fluorophore + 1) % (fluorophore_count+1);
                colour_fluorophore( *i );
            }
        }
        DEBUG("Processed notice_drawn_rectangle");
    }

void DisplayHandler::colour_fluorophore( const Tile& i )
{
    dStorm::display::Image orig_crop = crop( image, i.region_start, i.region_end );
    for ( dStorm::display::Image::const_iterator j = orig_crop.begin(); j != orig_crop.end(); ++j)
    {
        display::Image::Position pos = j.position();
        pos += i.region_start;
        display::PixelChange p( pos );
        p.color.red() = ( i.fluorophore == 0 || i.fluorophore == fluorophore_count )
            ? j->red() : 0;
        p.color.green() = ( i.fluorophore == 1 || i.fluorophore == fluorophore_count )
            ? j->green() : 0;
        p.color.blue() = ( i.fluorophore == 2 || i.fluorophore == fluorophore_count )
            ? j->blue() : 0;
        next_change->change_pixels.push_back( p );
    }
}

GUI::GUI( boost::ptr_vector<Tile>& w, const Input& input, Engine& engine, simparm::NodeHandle ui ) 
: input(input), engine(engine.block()), ui(ui)
{
    DEBUG("Constructed GUI " << this);
    if ( w.size() > tiles_per_view() )
        work.transfer( work.end(), w.begin(), w.begin() + tiles_per_view(), w );
    else
        work.transfer( work.end(), w.begin(), w.end(), w );
}

GUI::~GUI() {
    DEBUG("Destructing GUI " << this);
}

struct deselected : public std::unary_function<bool,const Tile>
{
    const int fluorophore_count;
    deselected( int fluorophore_count ) : fluorophore_count(fluorophore_count) {}
    bool operator()( const Tile& t ) const {
        return t.fluorophore >= fluorophore_count || t.fluorophore < 0;
    }
};

boost::ptr_vector<Tile> GUI::let_user_select()
{
    DEBUG("Running GUI " << this);
    assert( input.fluorophore_count > 0 );
    DisplayHandler handler( make_spot_display(), work, input.fluorophore_count, ui );
    handler.wait_for_destruction();

    work.erase_if( deselected(input.fluorophore_count) );
    DEBUG("Finished running GUI " << this);
    return work;
}

dStorm::engine::Image2D::Size GUI::get_maximum_tile_size() 
{
    dStorm::engine::Image2D::Size rv = dStorm::engine::Image2D::Size::Constant( 1 * camera::pixel );
    for ( boost::ptr_vector<Tile>::iterator i = work.begin(); i != work.end(); ++i )
    {
        i->bounds.clear();
        for (int p = 0; p < i->image.plane_count(); ++p) {
            traits::Projection::ROISpecification roi( i->spot.position().head<2>(), input.width );
            i->bounds.push_back( input.traits->plane(p).projection().get_region_of_interest(roi) );
            const traits::Projection::Bounds& b = i->bounds.back();
            for (Direction i = Direction_First; i != Direction_2D; ++i)
                rv[i] = std::max( b.width( i ), rv[i] );
        }
    }
    return rv;
}
    
std::auto_ptr< dStorm::display::Change > 
GUI::make_spot_display() {
    typedef dStorm::Image< dStorm::Pixel, display::Image::Dim > TargetImage;
    dStorm::engine::Image2D::Size selection_size = get_maximum_tile_size();
    TargetImage::Size tile_size, image_size;
    const quantity<camera::length,int> plane_height = (selection_size.y() + 1 * camera::pixel);

    tile_size.fill( 1 * camera::pixel );
    image_size.fill( 1 * camera::pixel );
    tile_size.x() = (selection_size.x() + 2 * camera::pixel);
    image_size.x() = tile_size.x() * tile_cols;
    tile_size.y() = (plane_height * input.traits->plane_count() + 1 * camera::pixel);
    image_size.y() = tile_size.y() * tile_rows;
    DEBUG("Tile size is " << tile_size.transpose() << " " << image_size.transpose());

    TargetImage normalized( image_size );
    normalized.fill( 0 );

    std::vector< dStorm::Image<engine::StormPixel,2>::PixelPair > ranges;
    for ( boost::ptr_vector<Tile>::iterator i = work.begin(); i != work.end(); ++i )
    {
        const int d = i->image.plane_count();

        for (int j = 0; j < d; ++j) {
            dStorm::Image<engine::StormPixel,2>::PixelPair local 
                = i->image.plane( j ).minmax();
            if ( int(ranges.size()) <= j ) {
                ranges.push_back( local );
                assert( int(ranges.size()) == j + 1 );
            } else {
                ranges[j].first = std::min( ranges[j].first, local.first );
                ranges[j].second = std::max( ranges[j].second, local.second );
            }
        }
    }

    quantity<camera::length,int> 
        current_line_top( 0 * camera::pixel ), current_left_edge( 0 * camera::pixel),
        current_plane_top( current_line_top );
    int current_col = 0;
    for ( boost::ptr_vector<Tile>::iterator i = work.begin(); i != work.end(); ++i )
    {
        DEBUG("Tile " << i->spot.position().transpose());
        assert( ! i->image.has_invalid_planes() );
        current_plane_top = current_line_top;
        for (int p = 0; p < i->image.plane_count(); ++p )
        {
            assert( int(i->bounds.size()) > p );
            traits::Projection::Bounds b = i->bounds[p];
            engine::Image2D::Size from_lower, from_upper;
            TargetImage::Size to_lower, to_upper;
            from_lower.fill( p * camera::pixel ); from_upper = from_lower;
            from_lower = b.lower_corner();
            from_upper = b.upper_corner();
            to_lower.x() = current_left_edge + 1 * camera::pixel;
            to_lower.y() = current_plane_top + 1 * camera::pixel;
            to_upper.head<2>() = (from_upper - from_lower).head<2>() + to_lower.head<2>();
            if ( p == 0 ) i->region_start = to_lower; 
            if ( p == i->image.plane_count()-1 ) i->region_end = to_upper;
            engine::Image2D cropped = crop( i->image.plane(p), from_lower, from_upper );
            DEBUG("Transform-Cropping " << to_lower.transpose() << " to " << 
                  to_upper.transpose() << " from " << normalized.sizes().transpose());
            TargetImage target_window = crop( normalized, to_lower, to_upper );
            DEBUG("Sizes are " << cropped.size_in_pixels() << " and " << target_window.size_in_pixels() );
            assert( cropped.size_in_pixels() <= target_window.size_in_pixels() );
            std::transform( cropped.begin(), cropped.end(), target_window.begin(), 
                normalize( ranges[p], std::make_pair( 0, 255 ) ) );

            current_plane_top += plane_height;
        }
        current_left_edge += tile_size.x();
        ++current_col;
        if ( current_col >= tile_cols ) {
            current_col = 0;
            current_left_edge = 0 * camera::pixel;
            current_line_top += tile_size.y();
        }
    }

    std::auto_ptr<dStorm::display::Change> c( new dStorm::display::Change(0));
    c->do_clear = true;
    c->clear_image.background = dStorm::Pixel::Black();
    c->do_resize = true;
    c->resize_image.size = normalized.sizes();
    for (int i = 0; i < int(ranges.size()); ++i)
        c->resize_image.keys.push_back( display::KeyDeclaration("ADC", "A/D counts per pixel", 256) );
    c->do_change_image = true;
    c->image_change.new_image = normalized;
    std::transform( ranges.begin(), ranges.end(),
                    std::back_inserter( c->changed_keys ),
                    &display::KeyChange::make_linear_key );
    return c;
}

}
}

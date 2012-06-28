#ifndef DSTORM_VIEWER_DISPLAY_H
#define DSTORM_VIEWER_DISPLAY_H

#include "ImageDiscretizer.h"
#include <vector>
#include <dStorm/display/Manager.h>
#include <dStorm/Engine.h>

namespace dStorm {
namespace viewer {

class ColourScheme;
class LiveCache;
class Config;
class Status;

class Display 
: public DiscretizationListener
{
private:
    typedef Discretizer< LiveCache > MyDiscretizer;
    MyDiscretizer& discretizer;
    const ColourScheme& colorizer;

    std::vector< bool > ps;
    int ps_step[ Im::Dim - 1 ];

    display::WindowProperties props;
    display::DataSource& vph;

    std::auto_ptr<display::Change> next_change;
    std::auto_ptr<display::WindowHandle> window_id;
    std::auto_ptr<dStorm::EngineBlock> termination_block;

    boost::optional<display::ResizeChange> my_size;
    const Status& status;
    simparm::NodeHandle current_ui;

    void setSize( const dStorm::display::ResizeChange& size );
    inline std::vector<bool>::reference is_on( const Im::Position& );

public:
    Display( 
        MyDiscretizer& disc, 
        const Status& config,
        dStorm::display::DataSource& vph,
        const ColourScheme& colorizer,
        std::auto_ptr<dStorm::display::Change> initial_state
            = std::auto_ptr<dStorm::display::Change>()
    );
    void setSize(const MetaInfo& traits);

    inline void pixelChanged( const Im::Position& );
    void clean(bool);
    void clear(); 
    inline void notice_key_change( int index, Pixel pixel, float value );

    std::auto_ptr<dStorm::display::Change> get_changes();

    void save_image(std::string filename, const Config&);

    const boost::optional<dStorm::display::ResizeChange>&
        getSize() const { return my_size; }
    void show_window();

    void set_job_name( const std::string& name ) { props.name = name; }
};

std::vector<bool>::reference Display::is_on( const Im::Position& i )
{
    int offset = i[0].value();
    for (int j = 1; j < Im::Dim; ++j)
        offset += ps_step[j-1] * i[j].value();
    return ps[ offset ];
}

void Display::pixelChanged( const Im::Position& p ) {
    std::vector<bool>::reference is_on = this->is_on( p );
    if ( ! is_on ) {
        next_change->change_pixels.push_back( dStorm::display::PixelChange(p) );
        /* The color field will be set when the clean handler
            * runs. */
        is_on = true;
    }
}

void Display::notice_key_change( int index, 
        Pixel pixel, float value )
{
    next_change->changed_keys.front().push_back( dStorm::display::KeyChange(
        index, pixel, value ) );
}

}
}

#endif

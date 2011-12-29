#ifndef DSTORM_VIEWER_DISPLAY_H
#define DSTORM_VIEWER_DISPLAY_H

#include "LiveCache.h"
#include "ImageDiscretizer.h"
#include "Config_decl.h"
#include "Status_decl.h"
#include <vector>
#include <dStorm/display/Manager.h>

namespace dStorm {
namespace viewer {

class BaseDisplay {
    std::vector< bool > ps;
    int ps_step[ display::Image::Dim - 1 ];
  public:
    void setSize( const display::Image::Size& size );
    inline std::vector<bool>::reference
        is_on( const display::Image::Position& );
    void clear();
};

template <typename UsedColorizer>
class Display 
    : public DiscretizationListener,
      private BaseDisplay
{
  public:
    typedef UsedColorizer Colorizer;
    typedef Discretizer< LiveCache<Display> > MyDiscretizer;

  private:
    MyDiscretizer& discretizer;
    const Colorizer& colorizer;

    dStorm::display::Manager::WindowProperties props;
    dStorm::display::DataSource& vph;

    std::auto_ptr<dStorm::display::Change> next_change;
    std::auto_ptr<dStorm::display::Manager::WindowHandle> window_id;

    boost::optional<dStorm::display::ResizeChange> my_size;

    void setSize( const dStorm::display::ResizeChange& size );

  public:
    Display( 
        MyDiscretizer& disc, 
        const Config& config,
        dStorm::display::DataSource& vph,
        const Colorizer& colorizer,
        std::auto_ptr<dStorm::display::Change> initial_state
            = std::auto_ptr<dStorm::display::Change>()
    );
    void setSize(const input::Traits< Image<int,2> >& traits);

    inline void pixelChanged( const display::Image::Position& );
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


}
}

#endif

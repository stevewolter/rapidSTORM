#ifndef DSTORM_VIEWER_DISPLAY_H
#define DSTORM_VIEWER_DISPLAY_H

#include "LiveCache.h"
#include "ImageDiscretizer.h"
#include "Config_decl.h"
#include "Status_decl.h"
#include <vector>
#include <dStorm/helpers/DisplayManager.h>

namespace dStorm {
namespace viewer {

template <typename UsedColorizer>
class Display 
    : public DiscretizationListener
{
  public:
    typedef UsedColorizer Colorizer;
    typedef Discretizer< LiveCache<Display> > MyDiscretizer;

  private:
    MyDiscretizer& discretizer;
    typedef std::vector< bool > PixelSet;
    PixelSet ps;
    int ps_step;

    bool do_show_window;
    dStorm::Display::Manager::WindowProperties props;
    dStorm::Display::DataSource& vph;

    std::auto_ptr<dStorm::Display::Change> next_change;
    std::auto_ptr<dStorm::Display::Manager::WindowHandle> window_id;

    dStorm::Display::ResizeChange my_size;

  public:
    Display( 
        MyDiscretizer& disc, 
        const Config& config,
        dStorm::Display::DataSource& vph 
    );
    void setSize(const input::Traits< Image<int,2> >& traits);

    inline void pixelChanged(int x, int y);
    void clean(bool);
    void clear(); 
    inline void notice_key_change( int index, Pixel pixel, float value );

    std::auto_ptr<dStorm::Display::Change> get_changes();

    void save_image(std::string filename,
                    bool with_key);

    dStorm::Display::ResizeChange getSize() const { return my_size; }
};


}
}

#endif

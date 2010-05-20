#ifndef DSTORM_VIEWER_DISPLAY_H
#define DSTORM_VIEWER_DISPLAY_H

#include "ImageDiscretizer.h"
#include "ViewerConfig.h"
#include <vector>
#include <dStorm/helpers/DisplayManager.h>

namespace dStorm {
namespace viewer {

template <typename Colorizer>
class Display 
    : public DiscretizedImage::Listener
{
    typedef DiscretizedImage::ImageDiscretizer<Colorizer,Display>
        Discretizer;

    Discretizer& discretizer;
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
        Discretizer& disc, 
        const Viewer::_Config& config,
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
};


}
}

#endif

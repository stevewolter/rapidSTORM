#ifndef DSTORM_TEXT_STREAM_UI_WINDOW_H
#define DSTORM_TEXT_STREAM_UI_WINDOW_H

#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include "simparm/Entry.h"
#include "simparm/TriggerEntry.h"
#include <boost/thread/recursive_mutex.hpp>
#include "display/DataSource.h"
#include "display/Manager.h"

namespace simparm {
namespace text_stream {
namespace image_window {

class MainThread;

class Window : public boost::enable_shared_from_this<Window>
{
    MainThread& m;
    boost::shared_ptr<dStorm::display::DataSource> handler;
public:
    typedef dStorm::display::Image Image;
    Image current_display;
    dStorm::display::Change state;
    int number;

private:
    simparm::Object window_object;
    simparm::Entry<std::string> digest;
    simparm::Entry<float> mean;
    simparm::Entry<int> nonzero_count;
    simparm::Entry<int> frame_number;
    simparm::Entry<float> key_31;
    simparm::Entry<int> window_width;
    simparm::Entry<unsigned long> which_key;
    simparm::Entry<unsigned long> top, bottom, left, right;
    simparm::StringEntry new_limit;
    simparm::TriggerEntry close_, set_lower_limit, set_upper_limit, draw_rectangle_;
    simparm::BaseAttribute::ConnectionStore listening[4];

    class GUINode;
    std::auto_ptr< GUINode > gui_node;

public:
    Window( MainThread& m,
            const dStorm::display::WindowProperties& properties,
            boost::shared_ptr<dStorm::display::DataSource> source,
            int number);
    ~Window();
    void handle_resize( 
        const dStorm::display::ResizeChange& );
    bool get_and_handle_change();

    void update_window();
    void attach_ui( simparm::NodeHandle at );

    void handle_disassociation();
    void save_window( const dStorm::display::SaveRequest& );

private:
    void print_status();
    void close_window();
    void notice_lower_limit();
    void notice_upper_limit();
    void notice_drawn_rectangle();

    void draw_rectangle( int l, int r, int t, int b );
    void set_key_limit( int key, bool lower_limit, std::string new_limit );
    void close();
};

}
}
}

#endif

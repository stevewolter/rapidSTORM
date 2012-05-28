#ifndef DSTORM_TEXT_STREAM_UI_WINDOW_H
#define DSTORM_TEXT_STREAM_UI_WINDOW_H

#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <simparm/Entry.h>
#include <simparm/TriggerEntry.h>
#include <boost/thread/recursive_mutex.hpp>
#include <dStorm/display/DataSource.h>
#include <dStorm/display/Manager.h>

namespace dStorm {
namespace text_stream_ui {

class Manager;

class Window : public boost::enable_shared_from_this<Window>
{
    Manager& m;
    boost::recursive_mutex handler_mutex;
    display::DataSource* handler;
public:
    typedef display::Image Image;
    Image current_display;
    display::Change state;
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
    Window( Manager& m,
            const display::Manager::WindowProperties& properties,
            display::DataSource& source,
            int number);
    ~Window();
    void handle_resize( 
        const display::ResizeChange& );
    bool get_and_handle_change();

    void attach_ui( simparm::NodeHandle at );
    void print_status(bool force_print = false);

    void drop_handler() {
        get_and_handle_change();
        boost::lock_guard< boost::recursive_mutex > lock( handler_mutex );
        handler = NULL;
    }

    void handle_disassociation();
    void save_window( const display::SaveRequest& );

private:
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

#endif

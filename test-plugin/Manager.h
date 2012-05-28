#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include <dStorm/display/Manager.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <dStorm/stack_realign.h>
#include <simparm/Object.h>

namespace dStorm {
namespace text_stream_ui {

class Window;

class Manager 
: public display::Manager
{
    class Handle;

    std::auto_ptr<WindowHandle>
        register_data_source_impl
        (const WindowProperties& properties,
         display::DataSource& handler);

    boost::mutex source_list_mutex;
    boost::mutex request_mutex;
    boost::condition gui_run;
    bool running;
    int number;

    simparm::Object master_object;
    simparm::NodeHandle current_ui;

    std::vector< boost::shared_ptr<Window> > sources_queue;
    typedef std::vector< boost::function0<void> > Requests;
    Requests requests;
    
    std::auto_ptr<display::Manager>
        previous;
    boost::thread ui_thread;

    boost::shared_ptr<Window> next_source();
    void dispatch_events();
    DSTORM_REALIGN_STACK void run();

    void heed_requests();

    void store_image_impl( const display::StorableImage& );

  public:
    Manager(display::Manager *p);
    Manager(const Manager&);
    ~Manager();

    void attach_ui( simparm::NodeHandle );

    void stop() {}
    void request_action( boost::function0<void> );

    void remove_window_from_event_queue( boost::shared_ptr<Window> );
};

}
}

#endif

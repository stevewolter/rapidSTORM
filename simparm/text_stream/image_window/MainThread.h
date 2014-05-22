#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include "display/Manager.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include "stack_realign.h"
#include "simparm/Object.h"
#include "simparm/Entry.h"

namespace simparm {
namespace text_stream {
namespace image_window {

class Window;

class MainThread 
{
    class Handle;

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
    
    boost::thread ui_thread;

    boost::shared_ptr<Window> next_source();
    void dispatch_events();
    DSTORM_REALIGN_STACK void run();

    void heed_requests();

  public:
    MainThread();
    MainThread(const MainThread&);
    ~MainThread();

    void attach_ui( simparm::NodeHandle );

    void stop() {}
    void request_action( boost::function0<void> );
    void print( std::string ) const;

    void remove_window_from_event_queue( boost::shared_ptr<Window> );

    std::auto_ptr<dStorm::display::WindowHandle>
        register_data_source
        (const dStorm::display::WindowProperties& properties,
         dStorm::display::DataSource& handler);

};

}
}
}

#endif

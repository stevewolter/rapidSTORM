#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/helpers/thread.h>
#include <dStorm/Job.h>
#include <boost/utility.hpp>
#include <map>
#include <list>
#include <Eigen/Core>

namespace Eigen {

template <> class NumTraits<dStorm::Pixel>
    : public NumTraits<int> {};

}

class Manager 
: public dStorm::Display::Manager, private ost::Thread
{
    class Handle;
    class ControlConfig;

    std::auto_ptr<WindowHandle>
        register_data_source
        (const WindowProperties& properties,
         dStorm::Display::DataSource& handler);
    void run_in_GUI_thread( ost::Runnable* code );

    ost::Mutex mutex;
    ost::Condition gui_run;
    typedef ost::MutexLock guard;
    bool running;
    int number;

    std::auto_ptr<ControlConfig> control_config;

    struct Source {
        typedef dStorm::Image<dStorm::Pixel,2> Image;
        dStorm::Display::DataSource& handler;
        Image current_display;
        dStorm::Display::Change state;
        int number;
        bool wants_closing, may_close;

        Source( const WindowProperties& properties,
                dStorm::Display::DataSource& source,
                int number);
        void handle_resize( 
            const dStorm::Display::ResizeChange& );
        bool get_and_handle_change();
    };
    typedef std::list<Source> Sources;
    Sources sources;
    typedef std::map<int, Sources::iterator> SourcesMap;
    SourcesMap sources_by_number;
    
    std::auto_ptr<dStorm::Display::Manager>
        previous;

    void dispatch_events();
    void run();

    void print_status(Source& source, std::string prefix, bool force_print = false);

  public:
    Manager(dStorm::Display::Manager *p);
    Manager(const Manager&);
    ~Manager();

    simparm::Node* getConfig();

    void store_image(
        std::string filename,
        const dStorm::Display::Change& image);

    void stop() {}
};

#endif

#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/helpers/thread.h>
#include <boost/utility.hpp>
#include <set>
#include <list>
#include <Eigen/Core>

namespace Eigen {

template <> class NumTraits<dStorm::Pixel>
    : public NumTraits<int> {};

}

class Manager : public dStorm::Display::Manager, private ost::Thread {
    class Handle;

    std::auto_ptr<WindowHandle>
        register_data_source
        (const WindowProperties& properties,
         dStorm::Display::DataSource& handler);
    void run_in_GUI_thread( ost::Runnable* code );

    ost::Mutex mutex;
    ost::Condition gui_run;
    typedef ost::MutexLock guard;
    bool running;

    struct Source {
        typedef Eigen::Matrix<dStorm::Pixel,
                              Eigen::Dynamic,
                              Eigen::Dynamic> Image;
        dStorm::Display::DataSource& handler;
        Image current_display;
        dStorm::Display::Change state;

        Source( const WindowProperties& properties,
                dStorm::Display::DataSource& source );
        void handle_resize( 
            const dStorm::Display::ResizeChange& );
        bool get_and_handle_change();
    };
    typedef std::list<Source> Sources;
    Sources sources;
    
    std::auto_ptr<dStorm::Display::Manager>
        previous;

    void dispatch_events();
    void run();

  public:
    Manager(dStorm::Display::Manager *p);
    Manager(const Manager&);
    ~Manager();

    void store_image(
        std::string filename,
        const dStorm::Display::Change& image);
};

#endif

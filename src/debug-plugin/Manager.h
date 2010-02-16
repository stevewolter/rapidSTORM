#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include <dStorm/helpers/DisplayManager.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <set>
#include <Eigen/Core>

namespace Eigen {

template <> class NumTraits<dStorm::Pixel>
    : public NumTraits<int> {};

}

class Manager : public dStorm::Display::Manager {
    class Handle;

    std::auto_ptr<WindowHandle>
        register_data_source
        (const WindowProperties& properties,
         dStorm::Display::DataSource& handler);
    void run_in_GUI_thread( ost::Runnable* code );

    boost::mutex mutex;
    typedef boost::lock_guard<
        boost::mutex> guard;
    boost::thread thread;
    bool running;

    struct Source {
        typedef Eigen::Matrix<dStorm::Pixel,
                              Eigen::Dynamic,
                              Eigen::Dynamic> Image;
        dStorm::Display::DataSource& handler;
        Image current_display;

        Source( const WindowProperties& properties,
                dStorm::Display::DataSource& source );
        void handle_resize( 
            const dStorm::Display::ResizeChange& );
        bool get_and_handle_change();
    };
    typedef std::list<Source> Sources;
    Sources sources;

    void dispatch_events();

  public:
    Manager();
    Manager(const Manager&);
    ~Manager();
};

#endif

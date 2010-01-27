#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include <dStorm/helpers/DisplayManager.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <set>

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
    bool was_started;

    typedef std::set<dStorm::Display::DataSource*>
        Sources;
    Sources sources;

    void dispatch_events();

  public:
    Manager();
    Manager(const Manager&);
};

#endif

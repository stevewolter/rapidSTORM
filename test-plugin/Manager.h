#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include <dStorm/display/Manager.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <dStorm/image/constructors.h>
#include <dStorm/Job.h>
#include <boost/utility.hpp>
#include <map>
#include <set>
#include <Eigen/Core>

#include <dStorm/stack_realign.h>
#include <boost/optional/optional.hpp>

namespace Eigen {

template <> class NumTraits<dStorm::Pixel>
    : public NumTraits<int> {};

}

class Manager 
: public dStorm::display::Manager
{
    class Handle;
    class ControlConfig;

    std::auto_ptr<WindowHandle>
        register_data_source
        (const WindowProperties& properties,
         dStorm::display::DataSource& handler);

    struct Disassociation {
        bool commenced;
        boost::mutex m;
        boost::condition c;
        Disassociation() : commenced(false) {}
    };
    struct Close { };
    struct SetLimit { 
        bool lower_limit; int key; std::string limit;
        SetLimit(bool b, int key, std::string limit) : lower_limit(b), key(key), limit(limit) {} };
    struct DrawRectangle { int l, r, t, b; DrawRectangle(int l, int r, int t, int b) : l(l), r(r), t(t), b(b) {} };

    typedef boost::variant< Close, SetLimit, DrawRectangle, dStorm::display::SaveRequest, boost::reference_wrapper<Disassociation> > Request;

    boost::recursive_mutex mutex;
    boost::mutex request_mutex;
    boost::condition gui_run;
    bool running;
    int number;

    std::auto_ptr<ControlConfig> control_config;

    struct Source : public boost::static_visitor<void> 
    {
        typedef dStorm::display::Image Image;
        dStorm::display::DataSource& handler;
        Image current_display;
        dStorm::display::Change state;
        int number;
        bool wants_closing, may_close;

        Source( const WindowProperties& properties,
                dStorm::display::DataSource& source,
                int number);
        void handle_resize( 
            const dStorm::display::ResizeChange& );
        bool get_and_handle_change();

        void operator()( const Close&, Manager& m );
        void operator()( const SetLimit&, Manager& m );
        void operator()( const DrawRectangle&, Manager& m );
        void operator()( const dStorm::display::SaveRequest&, Manager& m );
        void operator()( Disassociation&, Manager& m );
    };
    typedef std::map<int, boost::shared_ptr<Source> > Sources;
    Sources sources;
    typedef std::vector< std::pair<boost::shared_ptr<Source>, Request> >
        Requests;
    Requests requests;
    
    std::auto_ptr<dStorm::display::Manager>
        previous;
    boost::thread ui_thread;

    void dispatch_events();
    DSTORM_REALIGN_STACK void run();

    void print_status(Source& source, std::string prefix, bool force_print = false);
    void heed_requests();

  public:
    Manager(dStorm::display::Manager *p);
    Manager(const Manager&);
    ~Manager();

    simparm::Node* getConfig();

    void store_image(
        std::string filename,
        const dStorm::display::Change& image);

    void stop() {}
    void request_action( boost::shared_ptr<Source>& on, const Request& request );
};

#endif

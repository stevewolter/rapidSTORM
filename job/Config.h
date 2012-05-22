#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/Config.h>
#include <dStorm/output/Config.h>
#include <dStorm/input/Link.h>
#include <memory>
#include <list>
#include <simparm/Set.hh>
#include <simparm/Menu.hh>
#include <simparm/Callback.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Entry.hh>
#include <simparm/NodeHandle.hh>
#include <boost/ptr_container/ptr_list.hpp>
#include <dStorm/JobMaster.h>

namespace dStorm {
namespace output { class OutputSource; }
namespace job {

/** Configuration that summarises all
    *  configuration items offered by the dStorm library. */
class Config : public dStorm::Config
{
  private:
    friend class EngineChoice;
    class TreeRoot;
    class InputListener;

    simparm::Set car_config;
    simparm::NodeHandle current_ui;

    std::auto_ptr<TreeRoot> outputRoot;
    std::auto_ptr<input::Link> input;
    input::Link::Connection input_listener;

    void traits_changed( boost::shared_ptr<const input::MetaInfo> );
    void create_input( std::auto_ptr<input::Link> );

  public:
    Config();
    Config(const Config &c);
    ~Config();
    Config *clone() const { return new Config(*this); }

    void attach_ui( simparm::Node& at );
    //void processCommand( std::istream& i ) { current_ui->processCommand(i); }
    void send( simparm::Message& m ) { current_ui->send(m); }
    //std::list<std::string> printValues() { return current_ui->printValues(); }

    simparm::Node& user_interface_handle() { return *current_ui; }

    output::OutputSource& outputSource;
    output::Config& outputConfig;

    simparm::Menu helpMenu;
    simparm::Set outputBox;
    simparm::FileEntry configTarget;
    simparm::BoolEntry auto_terminate;
    /** Number of parallel computation threads to run. */
    simparm::Entry<unsigned long> pistonCount;

    void add_input( std::auto_ptr<input::Link>, InsertionPlace );
    void add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> );
    void add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> );
    void add_output( std::auto_ptr<output::OutputSource> );

    const input::MetaInfo& get_meta_info() const;
    std::auto_ptr<input::BaseSource> makeSource() const;

    void all_modules_loaded();
    void create_and_run( JobMaster& );
};
}
}

#endif

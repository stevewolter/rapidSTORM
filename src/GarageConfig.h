#ifndef GARAGECONFIG_H
#define GARAGECONFIG_H

#include "engine/CarConfig.h"
#include "InputStream.h"
#include <dStorm/output/Config.h>
#include <simparm/TriggerEntry.hh>
#include <dStorm/data-c++/AutoList.h>
#include <set>
#include <string>
#include <boost/utility.hpp>

namespace dStorm {

class GarageConfig 
: boost::noncopyable,
  private simparm::Node::Callback
{
    std::auto_ptr<dStorm::engine::CarConfig> carConfig;
    simparm::Attribute<std::string> help_file;
    simparm::TriggerEntry run;

    void operator()(const simparm::Event&) ;

    std::set<std::string> avoid_auto_filenames;

    GarageConfig& operator=(const GarageConfig&) ;

    void registerNamedEntries() ;

  public:
    GarageConfig();
    GarageConfig(const GarageConfig& c) ;
    GarageConfig* clone() const { return new GarageConfig(*this); }
    ~GarageConfig();

    void set_input_file( const std::string& );
    void run_job();

    engine::CarConfig& getCarConfig() { return *carConfig; }
    const engine::CarConfig& getCarConfig() const { return *carConfig; }

    input::Config& get_input_config() { return carConfig->inputConfig; }
};

}

#endif

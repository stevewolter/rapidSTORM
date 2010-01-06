#ifndef GARAGECONFIG_H
#define GARAGECONFIG_H

#include "engine/CarConfig.h"
#include <dStorm/output/Config.h>
#include <simparm/TriggerEntry.hh>
#include <dStorm/data-c++/AutoList.h>
#include <set>
#include <string>
#include "MasterConfig.h"
#include <boost/utility.hpp>

namespace dStorm {

class GarageConfig 
: private simparm::Node::Callback
{
    MasterConfig::Ptr master;
    std::auto_ptr<dStorm::engine::CarConfig> carConfig;
    simparm::TriggerEntry externalControl, showTransmissionTree, run;

    void operator()(simparm::Node&, Cause, simparm::Node *) throw();

    std::set<std::string> avoid_auto_filenames;

    GarageConfig& operator=(const GarageConfig&) throw();

    void registerNamedEntries() throw();

  public:
    GarageConfig() throw();
    GarageConfig(const GarageConfig& c) throw();
    GarageConfig* clone() const { return new GarageConfig(*this); }
    ~GarageConfig();

    void set_input_file( const std::string& );
    void run_job();

    std::auto_ptr<simparm::Set> make_set();
    input::Config& get_input_config() { return carConfig->inputConfig; }
};

}

#endif

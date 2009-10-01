#ifndef GARAGECONFIG_H
#define GARAGECONFIG_H

#include <dStorm/CarConfig.h>
#include <simparm/TriggerEntry.hh>
#include <data-c++/AutoList.h>

class LibraryHandle;
class ModuleHandler {
    data_cpp::auto_list<LibraryHandle> lib_handles;
    std::auto_ptr<dStorm::OutputFactory> constructed_tcf;
    dStorm::OutputFactory* tcf;
  public:
    ModuleHandler( dStorm::OutputFactory* provided_tcf );
    ModuleHandler( const ModuleHandler& );
    ~ModuleHandler();

    dStorm::OutputFactory& get_tcf() { return *tcf; }
    void add_input_modules( CImgBuffer::Config& input_config );
};

class GarageConfig 
: public simparm::Set,
  private simparm::Node::Callback
{
    void operator()(simparm::Node&, Cause, simparm::Node *) throw();

    std::set<std::string> avoid_auto_filenames;
    ModuleHandler modules;

  public:
    dStorm::CarConfig carConfig;
    simparm::BoolEntry externalControl;
    simparm::TriggerEntry showTransmissionTree, run;

    GarageConfig(dStorm::TransmissionSourceFactory* tc = NULL) throw();
    GarageConfig(const GarageConfig& c) throw();

    void registerNamedEntries() throw();
};

#endif

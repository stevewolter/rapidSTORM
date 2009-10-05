#ifndef GARAGECONFIG_H
#define GARAGECONFIG_H

#include <dStorm/CarConfig.h>
#include <dStorm/BasicOutputs.h>
#include <simparm/TriggerEntry.hh>
#include <data-c++/AutoList.h>
#include <set>
#include <string>

class LibraryHandle;
class ModuleHandler {
    data_cpp::auto_list<LibraryHandle> lib_handles;
    std::set<std::string> loaded;

    enum LoadResult { Loaded, Failure };
    LoadResult try_loading_module( const char *filename );
    static int lt_dlforeachfile_callback( 
        const char *filename, void* data );
  public:
    ModuleHandler();
    ModuleHandler( const ModuleHandler& );
    ~ModuleHandler();

    void add_input_modules( CImgBuffer::Config& input_config );
    void add_output_modules( dStorm::BasicOutputs& tcf );
};

class GarageConfig 
: public simparm::Set,
  private simparm::Node::Callback
{
    void operator()(simparm::Node&, Cause, simparm::Node *) throw();

    std::set<std::string> avoid_auto_filenames;
    std::auto_ptr<dStorm::BasicOutputs> tcf;

  public:
    std::auto_ptr<dStorm::CarConfig> carConfig;
    simparm::BoolEntry externalControl;
    simparm::TriggerEntry showTransmissionTree, run;

    GarageConfig(ModuleHandler& module_handler) throw();
    GarageConfig(const GarageConfig& c) throw();
    GarageConfig* clone() const { return new GarageConfig(*this); }

    void registerNamedEntries() throw();
};

#endif

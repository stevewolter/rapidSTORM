#ifndef GARAGECONFIG_H
#define GARAGECONFIG_H

#include "engine/CarConfig.h"
#include <dStorm/output/Config.h>
#include <simparm/TriggerEntry.hh>
#include <dStorm/data-c++/AutoList.h>
#include <set>
#include <string>

class LibraryHandle;
class ModuleHandler {
    typedef data_cpp::auto_list<LibraryHandle> List;
    List lib_handles;
    std::set<std::string> loaded;

    enum LoadResult { Loaded, Failure };
    LoadResult try_loading_module( const char *filename );
    static int lt_dlforeachfile_callback( 
        const char *filename, void* data );
  public:
    ModuleHandler();
    ModuleHandler( const ModuleHandler& );
    ~ModuleHandler();

    void add_modules( dStorm::engine::CarConfig& input_config );
    std::string getDesc();
};

class GarageConfig 
: public simparm::Set,
  private simparm::Node::Callback
{
    void operator()(simparm::Node&, Cause, simparm::Node *) throw();

    std::set<std::string> avoid_auto_filenames;

  public:
    std::auto_ptr<dStorm::engine::CarConfig> carConfig;
    simparm::BoolEntry externalControl;
    simparm::TriggerEntry showTransmissionTree, run;

    GarageConfig(ModuleHandler& module_handler) throw();
    GarageConfig(const GarageConfig& c) throw();
    GarageConfig* clone() const { return new GarageConfig(*this); }

    void registerNamedEntries() throw();
};

#endif

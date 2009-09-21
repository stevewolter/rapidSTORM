#ifndef GARAGECONFIG_H
#define GARAGECONFIG_H

#include <dStorm/CarConfig.h>
#include <simparm/TriggerEntry.hh>

class GarageConfig 
: public simparm::Set,
  private simparm::Node::Callback
{
    void operator()(simparm::Node&, Cause, simparm::Node *) throw();

    std::set<std::string> avoid_auto_filenames;

  public:
    std::auto_ptr<dStorm::TransmissionSourceFactory> my_tc;
    dStorm::CarConfig carConfig;
    simparm::BoolEntry externalControl;
    simparm::TriggerEntry showTransmissionTree, run;

    GarageConfig(dStorm::TransmissionSourceFactory* tc = NULL) throw();
    GarageConfig(const GarageConfig& c) throw();

    void registerNamedEntries() throw();
};

#endif

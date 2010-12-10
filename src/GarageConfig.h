#ifndef GARAGECONFIG_H
#define GARAGECONFIG_H

#include <dStorm/CarConfig.h>
#ifdef INCLUDE_LOCPREC
#include <locprec/RegionSegmenterConfig.h>
#endif

class GarageConfig 
: public dStorm::CarConfig
#ifdef INCLUDE_LOCPREC
  , public locprec::RegionSegmenterConfig
#endif
  , private virtual simparm::ConfigNode::Callback
{
    void operator()(simparm::ConfigNode&, Cause, ConfigNode *) throw();
    void registerNamedEntries() throw();

  public:
#ifdef INCLUDE_LOCPREC
    simparm::ConfigEntryBool useLocprec;
#endif
    simparm::ConfigEntryBool externalControl;
    simparm::ConfigEntryTrigger run;

    GarageConfig(dStorm::TransmissionSourceFactory&tc ) throw();
    GarageConfig(const GarageConfig& c) throw();
};

#endif

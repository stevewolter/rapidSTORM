#ifndef TESTPLUGIN_VERBOSE_H
#define TESTPLUGIN_VERBOSE_H

#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>
#include <dStorm/log.h>

struct Verbose
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    class Source;

    Verbose(const Config& config) ;
    ~Verbose();
    Verbose* clone() const;

    AdditionalData announceStormSize(const Announcement& a) { 
        if (  a.resolution.is_set() ) {
            LOG( "Verbose plugin got announcement with "
                    << a.size.transpose() << " and size "
                    << *a.resolution );
            if ( a.speed.is_set() ) {
                LOG("Announced speed is " << *a.speed );
            }
        } else {
            LOG( "Verbose plugin got announcement with "
                    << a.size.transpose() );
        }
        return AdditionalData(); 
    }
    Result receiveLocalizations(const EngineResult& er) {
        LOG( "Verbose plugin got " << er.number << " localizations for " << er.forImage);
        return KeepRunning;
    }
    void propagate_signal(ProgressSignal s) {
        LOG("Verbose plugin got signal " << s);
    }

};

struct Verbose::_Config
 : public simparm::Object 
{
    _Config();
    void registerNamedEntries() {}
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

Verbose::_Config::_Config()
 : simparm::Object("Verbose", "Verbose")
{
}

class Verbose::Source
: public dStorm::output::OutputBuilder<Verbose> 
{
  public:
    Source( bool failSilently = false ) 
        : dStorm::output::OutputBuilder<Verbose>(failSilently) {}
    Source( const Source& o ) 
        : dStorm::output::OutputBuilder<Verbose>(o) {}

    Source *clone() const { return new Source(*this); }

    void set_output_file_basename(const dStorm::output::Basename& basename) {
        //LOG("Verbose plugin got basename " << basename.unformatted()());
    }
};

Verbose* Verbose::clone() const { 
    LOG( "Cloning verbose plugin" );
    return new Verbose(*this); 
}

Verbose::Verbose( const Config& config )
        : OutputObject("SegFault", "SegFault")
{
    LOG( "Constructed verbose plugin" );
}

Verbose::~Verbose()
{
    LOG( "Destroyed verbose plugin" );
}

#endif

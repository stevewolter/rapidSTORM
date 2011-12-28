#ifndef TESTPLUGIN_VERBOSE_H
#define TESTPLUGIN_VERBOSE_H

#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>
#include <dStorm/log.h>

template <typename Type>
inline std::ostream& operator<<( std::ostream& o, const boost::optional<Type>& v )
{
    if ( v.is_initialized() ) return (o << *v); else return (o << "(unset)");
}

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
        if (  a.position().resolution().x().is_initialized() ) {
            LOG( "Verbose plugin got announcement with "
                    << a.position().upper_limits().transpose() << " and size "
                    << (*a.position().resolution().x()) );
        } else {
            LOG( "Verbose plugin got announcement with "
                    << a.position().upper_limits().transpose() );
        }
        if ( a.image_number().resolution().is_initialized() ) {
            LOG("Announced speed is " << *a.image_number().resolution() );
        }
        return AdditionalData(); 
    }
    RunRequirements announce_run(const RunAnnouncement&) {
        LOG("Verbose plugin got signal Engine_is_restarted");
        return RunRequirements();
    }
    void receiveLocalizations(const EngineResult& er) {
        LOG( "Verbose plugin got " << er.size() << " localizations for " << er.forImage);
        if ( er.source.is_valid() ) {
            LOG( "Source image is attached with size " << er.source.sizes().transpose() );
        }
    }
    void store_results() {
        LOG("Verbose plugin got signal Engine_run_succeeded");
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

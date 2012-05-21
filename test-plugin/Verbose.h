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
: public dStorm::output::Output
{
    struct Config;

    Verbose(const Config& config) ;
    ~Verbose();
    Verbose* clone() const;

    AdditionalData announceStormSize(const Announcement& a) { 
        LOG( "Verbose plugin got announcement with "
                << a.position().upper_limits().transpose() );
        return AdditionalData(); 
    }
    RunRequirements announce_run(const RunAnnouncement&) {
        LOG("Verbose plugin got signal Engine_is_restarted");
        return RunRequirements();
    }
    void receiveLocalizations(const EngineResult& er) {
        LOG( "Verbose plugin got " << er.size() << " localizations for " << er.forImage);
        if ( er.source.is_initialized() && er.source->plane(0).is_valid() ) {
            LOG( "Source image is attached with size " << er.source->plane(0).sizes().transpose() );
        }
    }
    void store_results_(bool succeeded) {
        LOG("Verbose plugin got store_results signal, success " << succeeded);
    }

};

struct Verbose::Config
{
    Config() {}
    void attach_ui( simparm::Node& at ) {}
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
    static std::string get_name() { return "Verbose"; }
    static std::string get_description() { return "Verbose"; }
};

Verbose* Verbose::clone() const { 
    LOG( "Cloning verbose plugin" );
    return new Verbose(*this); 
}

Verbose::Verbose( const Config& config )
{
    LOG( "Constructed verbose plugin" );
}

Verbose::~Verbose()
{
    LOG( "Destroyed verbose plugin" );
}

#endif

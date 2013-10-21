#ifndef TESTPLUGIN_VERBOSE_H
#define TESTPLUGIN_VERBOSE_H

#include <simparm/Entry.h>
#include <simparm/Entry.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>
#include <boost/lexical_cast.hpp>

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
        log = "Verbose plugin got announcement";
        return AdditionalData(); 
    }
    RunRequirements announce_run(const RunAnnouncement&) {
        log = "Verbose plugin got signal Engine_is_restarted";
        return RunRequirements();
    }
    void receiveLocalizations(const EngineResult& er) {
        log = "Verbose plugin got " + boost::lexical_cast<std::string>( er.size() ) 
            + " localizations for " + boost::lexical_cast<std::string>( er.forImage );
        if ( er.source.is_initialized() && er.source->plane(0).is_valid() ) {
            log = "Source image is attached with size " +
                boost::lexical_cast<std::string>( er.source->plane(0).sizes().transpose() );
        }
    }
    void store_results_(bool succeeded) {
        log = "Verbose plugin got store_results signal, success " + boost::lexical_cast<std::string>( succeeded );
    }

private:
    simparm::Entry<std::string> log;
    virtual void attach_ui_( simparm::NodeHandle at ) { log.attach_ui(at); }
};

struct Verbose::Config
{
    Config() {}
    void attach_ui( simparm::NodeHandle at ) {}
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
    static std::string get_name() { return "Verbose"; }
    static std::string get_description() { return "Verbose"; }
    static simparm::UserLevel get_user_level() { return simparm::Debug; }
};

Verbose* Verbose::clone() const { 
    return new Verbose(*this); 
}

Verbose::Verbose( const Config& )
: log("VerboseLog", "Verbose plugin log", "")
{
}

Verbose::~Verbose()
{
}

#endif

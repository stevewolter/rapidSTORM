#ifndef TESTPLUGIN_VERBOSE_H
#define TESTPLUGIN_VERBOSE_H

#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>

struct Verbose
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<Verbose> Source;

    Verbose(const Config& config) ;
    ~Verbose();
    Verbose* clone() const;

    AdditionalData announceStormSize(const Announcement& a) { 
        std::cerr << "Verbose plugin got announcement with "
                  << a.traits.size.transpose() << " "
                  << a.traits.resolution.transpose() << "\n";
        return AdditionalData(); 
    }
    Result receiveLocalizations(const EngineResult& er) {
        std::cerr << "Verbose plugin got results for "
                  << er.forImage << "\n";
        return KeepRunning;
    }
    void propagate_signal(ProgressSignal s) {
        std::cerr << "Verbose plugin got signal " << s << "\n";
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

Verbose* Verbose::clone() const { 
    std::cerr << "Cloning verbose plugin\n";
    return new Verbose(*this); 
}

Verbose::Verbose( const Config& config )
        : OutputObject("SegFault", "SegFault")
{
    std::cerr << "Constructed verbose plugin\n";
}

Verbose::~Verbose()
{
    std::cerr << "Destroyed verbose plugin\n";
}

#endif

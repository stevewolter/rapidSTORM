#ifndef TESTPLUGIN_BASENAME_PRINTER_H
#define TESTPLUGIN_BASENAME_PRINTER_H

#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>

struct BasenamePrinter
: public dStorm::output::OutputObject
{
    struct Config;
    typedef dStorm::output::FileOutputBuilder<BasenamePrinter> Source;

    BasenamePrinter(const Config& config);
    BasenamePrinter* clone() const;

    AdditionalData announceStormSize(const Announcement& a) 
        { return AdditionalData(); }
    void receiveLocalizations(const EngineResult&) {}
    void store_results() {}

};

struct BasenamePrinter::Config
 : public simparm::Object ,
   simparm::Listener
{
    dStorm::output::BasenameAdjustedFileEntry outputFile;

    Config();
    Config(const Config&);
    ~Config();
    void registerNamedEntries();
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
    void operator()( const simparm::Event& ) {
        std::cerr << this << ": Displaying output file name "
                  << outputFile.unformatted_name() << "\n";
    }
};

#endif

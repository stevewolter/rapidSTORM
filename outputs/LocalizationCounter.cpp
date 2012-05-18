#include <dStorm/output/Output.h>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <iostream>
#include <fstream>
#include <memory>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/units/frame_count.h>

#include "LocalizationCounter.h"

namespace dStorm {
namespace output {

class LocalizationCounter : public OutputObject {
    private:
    int count;
    frame_count last_config_update, config_increment;
    simparm::Entry<unsigned long> update;
    std::auto_ptr< std::ofstream > print_count;

    /** Copy constructor not implemented. */
    LocalizationCounter(const LocalizationCounter&);
    /** Assignment not implemented. */
    LocalizationCounter& operator=(const LocalizationCounter&);

    void store_results_( bool success ) {
        update = count;
        if ( success && !this->isActive()) std::cout << count << "\n"; 
    }

    public:
    struct Config { 
        simparm::FileEntry output_file;

        Config(); 
        bool can_work_with(Capabilities) { return true; }
        void attach_ui( simparm::Node& at ) { output_file.attach_ui( at ); }
        static std::string get_name() { return "Count"; }
        static std::string get_description() { return "Count localizations"; }
    };

    LocalizationCounter(const Config &);
    LocalizationCounter* clone() const 
        { throw std::runtime_error("LC::clone Not implemented."); }

    RunRequirements announce_run(const RunAnnouncement&) {
        count = 0; 
        update = 0; 
        last_config_update = 0;

        return RunRequirements();
    }
    AdditionalData announceStormSize(const Announcement &a) {
        update.setUserLevel(simparm::Object::Beginner);
        push_back(update);
        config_increment = 10 * camera::frame;

        count = 0; 
        return AdditionalData();
    }
    void receiveLocalizations(const EngineResult& er) {
        count += er.size(); 
        if ( print_count.get() ) {
            *print_count << er.forImage.value() << " " << er.size() << std::endl;
        }
        if ( last_config_update + config_increment < er.forImage )
        {
            update = count;
            last_config_update = er.forImage;
        }
    }

};

LocalizationCounter::Config::Config()
: output_file("ToFile", "Write localization count to file")
{
    output_file.userLevel = Object::Intermediate;
}

LocalizationCounter::LocalizationCounter(const Config &c)
: OutputObject("LocCountStat", "Localization counting status"),
  count(0),
  last_config_update(0),
  update("LocalizationCount", 
         "Number of localizations found", 0)
{
    update.helpID = "#Count_Count";
    print_count.reset( new std::ofstream( c.output_file().c_str(), std::ios::out ) );
}

std::auto_ptr< output::OutputSource > make_localization_counter_source() {
    return std::auto_ptr< output::OutputSource >( new output::OutputBuilder< LocalizationCounter::Config, LocalizationCounter >() );
}

}
}

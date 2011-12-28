#ifndef DSTORM_LOCALIZATIONCOUNTER_H
#define DSTORM_LOCALIZATIONCOUNTER_H

#include <dStorm/output/Output.h>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <iostream>
#include <fstream>
#include <memory>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/units/frame_count.h>

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

      public:
        struct _Config : public simparm::Object { 
            simparm::FileEntry output_file;

            _Config(); 
            bool can_work_with(Capabilities) { return true; }
            void registerNamedEntries() { push_back( output_file ); }
        };
        typedef simparm::Structure<_Config> Config;
        typedef OutputBuilder<LocalizationCounter> Source;

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
        void store_results() {
            update = count;
            if (!this->isActive()) std::cout << count << "\n"; 
        }

    };
}
}
#endif


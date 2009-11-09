#ifndef DSTORM_OUTPUT_CONFIG_H
#define DSTORM_OUTPUT_CONFIG_H

#include "SourceFactory.h"
#include <simparm/ChoiceEntry.hh>

namespace dStorm {
namespace output {

    class OutputSource;
    class Config
    : public SourceFactory,
      public simparm::NodeChoiceEntry<OutputSource>
    {
      public:
        Config();
        Config( const Config& );
        virtual Config* clone() const;
        virtual std::auto_ptr<OutputSource> make_output_source();

        void addChoice(OutputSource *toAdd);

        void reset_state() { value = NULL; }
        BasenameResult set_output_file_basename(
            const std::string& new_basename, std::set<std::string>& avoid);
    };

}
}

#endif

#ifndef BASIC_TRANSMISSIONS_H
#define BASIC_TRANSMISSIONS_H

#include <dStorm/OutputFactory.h>
#include <simparm/ChoiceEntry.hh>

namespace dStorm {
    class OutputSource;
    class BasicOutputs
    : public OutputFactory,
      public simparm::NodeChoiceEntry<OutputSource>
    {
      public:
        BasicOutputs();
        virtual BasicOutputs* clone() const { return new BasicOutputs(); }
        virtual std::auto_ptr<OutputSource> make_output_source();

        void addChoice(OutputSource *toAdd);

        void reset_state() { value = NULL; }
        BasenameResult set_output_file_basename(
            const std::string& new_basename, std::set<std::string>& avoid);
    };
}

#endif

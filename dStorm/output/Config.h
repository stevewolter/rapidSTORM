#ifndef DSTORM_OUTPUT_CONFIG_H
#define DSTORM_OUTPUT_CONFIG_H

#include "SourceFactory.h"
#include <simparm/Object.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace output {

    class OutputSource;

    struct ChoiceConfig
      : public simparm::Object,
        public std::auto_ptr<OutputSource>
    {
        virtual std::string getName() const;

        ChoiceConfig( std::auto_ptr<OutputSource> );
        ChoiceConfig( const ChoiceConfig& );
        ChoiceConfig& operator=( const ChoiceConfig& );
        ~ChoiceConfig();

        ChoiceConfig* clone() const { return new ChoiceConfig(*this); }
      private:
        void registerNamedEntries();
    };

    class Config
    : public simparm::ManagedChoiceEntry<ChoiceConfig>,
      public SourceFactory,
      public simparm::Node::Callback
    {
        Capabilities my_capabilities;
      protected:
        void set_source_capabilities(Capabilities src_cap);
      public:
        Config();
        Config( const Config& );
        virtual Config* clone() const;
        virtual std::auto_ptr<OutputSource> make_output_source();
        ~Config();

        void addChoice(OutputSource *toAdd);

        void reset_state() { value = ""; }
        void operator()(const simparm::Event&);
    };

}
}

#endif

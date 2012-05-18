#ifndef DSTORM_OUTPUT_CONFIG_H
#define DSTORM_OUTPUT_CONFIG_H

#include "SourceFactory.h"
#include <simparm/Object.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include "OutputSource.h"
#include <simparm/BoostSignal.hh>

namespace dStorm {
namespace output {

    class Config
    : public SourceFactory
    {
        simparm::ManagedChoiceEntry<OutputSource> choice;
        Capabilities my_capabilities;
        simparm::BoostSignalAdapter source_available;
      protected:
        void set_source_capabilities(Capabilities src_cap);
      public:
        Config();
        virtual Config* clone() const;
        virtual std::auto_ptr<OutputSource> make_output_source();
        ~Config();

        void addChoice(OutputSource *toAdd);

        void attach_ui( simparm::Node& at );
        void notify_when_output_source_is_available( const Callback& );
    };

}
}

#endif

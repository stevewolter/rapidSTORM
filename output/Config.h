#ifndef DSTORM_OUTPUT_CONFIG_H
#define DSTORM_OUTPUT_CONFIG_H

#include "output/SourceFactory.h"
#include "simparm/Object.h"
#include "simparm/ChoiceEntry.h"
#include "simparm/ManagedChoiceEntry.h"
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include "output/OutputSource.h"
#include "helpers/default_on_copy.h"

namespace dStorm {
namespace output {

    class Config
    : public SourceFactory
    {
        simparm::ManagedChoiceEntry<OutputSource> choice;
        Capabilities my_capabilities;
        default_on_copy< boost::signals2::signal<void()> > source_available;
        simparm::BaseAttribute::ConnectionStore listening;
      protected:
        void set_source_capabilities(Capabilities src_cap);
      public:
        Config();
        virtual Config* clone() const;
        virtual std::auto_ptr<OutputSource> make_output_source();
        ~Config();

        void addChoice(OutputSource *toAdd);

        void attach_ui( simparm::NodeHandle at );
        void notify_when_output_source_is_available( const Callback& );
    };

}
}

#endif

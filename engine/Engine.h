#ifndef DSTORM_ENGINE_ENGINE_H
#define DSTORM_ENGINE_ENGINE_H

#include <simparm/BoostUnits.hh>
#include <memory>
#include <dStorm/helpers/thread.h>
#include <simparm/TriggerEntry.hh>
#include "Config_decl.h"
#include <dStorm/engine/Input_decl.h>
#include <dStorm/output/Traits_decl.h>
#include <boost/utility.hpp>
#include <dStorm/output/LocalizedImage_decl.h>
#include <dStorm/input/Source.h>
#include <dStorm/Engine.h>

namespace dStorm {
namespace engine {
   /** The Engine class performs the main computation in the
    *  dSTORM module. It forks several threads (called pistons),
    *  which request images from the Input, make the
    *  SpotFinder locate candidates in them, fit them with the
    *  SpotFitter and output the results to the Output. */
   class Engine
   : public input::Source<output::LocalizedImage>,
     public simparm::Object,
     public dStorm::Engine
   {
      public:
        typedef input::Source<Image> Input;
        typedef input::Source<output::LocalizedImage>::TraitsPtr TraitsPtr;
      private:
        typedef input::Source<output::LocalizedImage> Base;

        std::auto_ptr<Input> input;
        Input::TraitsPtr imProp;

        Config& config;
        ost::Mutex mutex;
        simparm::Entry<unsigned long> errors;

        class _iterator;
        simparm::Node& node() { return *this; }

      public:
        Engine(Config& config, std::auto_ptr<Input> input);
        virtual ~Engine();

        void dispatch(Messages m);
        Base::iterator begin();
        Base::iterator end();
        TraitsPtr get_traits(Wishes);

        BaseSource& upstream() { return *input; }
        Capabilities capabilities() const { return input->capabilities(); }

        static boost::shared_ptr< input::Traits<output::LocalizedImage> >
            convert_traits( Config&, const input::Traits<engine::Image>& );

        void restart();
        void stop();
        void repeat_results();
        bool can_repeat_results();
        void change_input_traits( std::auto_ptr< input::BaseTraits > );
        std::auto_ptr<EngineBlock> block() ;
   };
}
}

#endif

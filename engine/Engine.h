#ifndef DSTORM_ENGINE_ENGINE_H
#define DSTORM_ENGINE_ENGINE_H

#include <memory>
#include <dStorm/helpers/thread.h>
#include <simparm/TriggerEntry.hh>
#include <dStorm/engine/Config_decl.h>
#include <dStorm/engine/Input_decl.h>
#include <dStorm/output/Traits_decl.h>
#include <boost/utility.hpp>
#include <dStorm/output/LocalizedImage_decl.h>
#include <dStorm/input/Source.h>

namespace dStorm {
namespace engine {
   /** The Engine class performs the main computation in the
    *  dSTORM module. It forks several threads (called pistons),
    *  which request images from the Input, make the
    *  SpotFinder locate candidates in them, fit them with the
    *  SpotFitter and output the results to the Output. */
   class Engine
   : public input::Source<output::LocalizedImage>,
     public input::Filter,
     public simparm::Object
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
        simparm::UnsignedLongEntry errors;

        class _iterator;

      public:
        Engine(Config& config, std::auto_ptr<Input> input);
        virtual ~Engine();

        void dispatch(Messages m);
        Base::iterator begin();
        Base::iterator end();
        TraitsPtr get_traits();

        BaseSource& upstream() { return *input; }
        boost::ptr_vector<output::Output> additional_outputs();

        static boost::shared_ptr< input::Traits<output::LocalizedImage> >
            convert_traits( Config&, boost::shared_ptr< const input::Traits<engine::Image> > );
   };
}
}

#endif

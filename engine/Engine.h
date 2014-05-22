#ifndef DSTORM_ENGINE_ENGINE_H
#define DSTORM_ENGINE_ENGINE_H

#include "simparm/BoostUnits.h"
#include <memory>
#include <boost/thread/mutex.hpp>
#include "engine/Config_decl.h"
#include "engine/Input_decl.h"
#include "output/Traits_decl.h"
#include <boost/utility.hpp>
#include "output/LocalizedImage_decl.h"
#include "input/Source.h"
#include "base/Engine.h"
#include "engine/Config.h"
#include "engine/EngineThread.h"

namespace dStorm {
namespace engine {
   /** The Engine class performs the main computation in the
    *  dSTORM module. It forks several threads (called pistons),
    *  which request images from the Input, make the
    *  SpotFinder locate candidates in them, fit them with the
    *  SpotFitter and output the results to the Output. */
   class Engine
   : public input::Source<output::LocalizedImage>,
     public dStorm::Engine
   {
      public:
        typedef input::Source<ImageStack> Input;
        typedef input::Source<output::LocalizedImage>::TraitsPtr TraitsPtr;
      private:
        typedef input::Source<output::LocalizedImage> Base;

        std::auto_ptr<Input> input;
        Input::TraitsPtr imProp, last_run_meta_info;

        Config config;
        std::vector<std::unique_ptr<EngineThread>> work_horses;
        boost::mutex mutex;
        simparm::Entry<unsigned long> errors;

        void attach_ui_( simparm::NodeHandle );
        bool GetNext(int thread, output::LocalizedImage* target) OVERRIDE;
        void set_thread_count(int num_threads) OVERRIDE;

      public:
        Engine(const Config& config, std::auto_ptr<Input> input);
        virtual ~Engine();

        void dispatch(Messages m);
        TraitsPtr get_traits();

        BaseSource& upstream() { return *input; }

        static boost::shared_ptr< input::Traits<output::LocalizedImage> >
            convert_traits( Config&, const input::Traits<engine::ImageStack>& );

        void restart();
        void stop();
        void repeat_results();
        bool can_repeat_results();
        void change_input_traits( std::auto_ptr< input::BaseTraits > );
        std::auto_ptr<EngineBlock> block() ;
        std::auto_ptr<EngineBlock> block_termination() 
            { throw std::logic_error("Not implemented"); }

        void increment_error_count();
   };
}
}

#endif

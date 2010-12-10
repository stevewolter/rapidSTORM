#include <data-c++/List.h>
#include <data-c++/Vector.h>
#include <CImg.h>
#include "Transmission.h"
#include "TransmissionConfig.h"
#include <ConfigEntry.hh>
#include "TraceReducer.h"

namespace dStorm {
    class EmissionTracker : public Transmission {
      public:
        class _Config : public TransmissionForwarderSource {
          protected:
            _Config(TransmissionSourceFactory& tc) throw();
          public:
            simparm::ConfigEntryUnsignedLong allowBlinking;
        };
        class Config : public _Config {
          private:
            std::auto_ptr<TraceReducerConfig> reducer;

          public:
            Config(TransmissionSourceFactory& tc) throw() 
            : ConfigObject("EmissionTracker", "Emission tracking configuration"),
              _Config(tc)
            {
                reducer = TraceReducerConfig::factory();
                ConfigNode::push_back( *reducer );
            }
            Config(const Config& o) throw() 
            : ConfigObject(o), _Config(o),
              reducer( o.reducer->clone() )
            {
                ConfigNode::push_back( *reducer );
            }
            Config* clone() const { return new Config(*this); }
            ~Config() throw() {}

            virtual std::auto_ptr<Transmission> makeTransmission() throw() {
                assert( output.get() != NULL );
                return std::auto_ptr<Transmission>( new EmissionTracker(
                    output->makeTransmission(), reducer->make_trace_reducer() ) );
            }
        };

      private:
        ost::Mutex mutex;
        typedef data_cpp::List<Trace> List;
        int image_width, image_height, movie_length;

        int next_to_track, track_modulo;
        ost::Condition next_to_track_moved;

        List::Allocator allocator;
        List output, garbage_bin;

        struct TrackingInformation {
            int imageNumber;
            cimg_library::CImg<List::iterator> positional;
            List emissions;

            TrackingInformation(List::Allocator &a, int width, int height)
            : positional(width, height), emissions(a)
                { imageNumber = -5; }

            void prepare(int imageNumber) throw() {
                this->imageNumber = imageNumber;
                memset(positional.ptr(), 0, 
                       positional.size() * sizeof(List::iterator));
            }
        };

        std::vector<TrackingInformation> tracking;

        inline List::iterator getNewEmissionNode() throw() {
            if (garbage_bin.empty())
                garbage_bin.push_back_uninitialized();
            return garbage_bin.begin();
        }

        void finalizeImage(int i) throw();

        class LocalizationCollater;
        std::auto_ptr<TraceReducer> reducer;
        std::auto_ptr<LocalizationCollater> collater;
        std::auto_ptr<Transmission> target;

      public:
        EmissionTracker( std::auto_ptr<Transmission> output,
                         std::auto_ptr<TraceReducer> reducer );
        ~EmissionTracker() throw();

        AdditionalData announceStormSize(int w, int h, int l) 
            throw(std::exception) 
        {
            ost::MutexLock lock(mutex);
            image_width = w; image_height = h; movie_length = l; 
            next_to_track = 0;
            tracking.resize(movie_length, 
                TrackingInformation(allocator, w, h) );
            AdditionalData childData = target->announceStormSize(w,h,l);
            if ( childData == NoData )
                throw std::runtime_error("Additional data requested from "
                    "emission tracker, but it can only provide "
                    "localizations.");
            return NoData;
        }

        Result receiveLocalizations(const EngineResult &) throw();

        virtual void deleteAllResults() throw() {
            ost::MutexLock lock(mutex);
            next_to_track = 0;
            target->deleteAllResults();
        }
        virtual void finish() throw() {
            ost::MutexLock lock(mutex);
            finalizeImage(next_to_track-1); target->finish(); 
        }

        virtual const char *getName() throw() { return "EmissionTracker"; }
    };
}

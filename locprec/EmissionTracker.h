#include <data-c++/List.h>
#include <data-c++/Vector.h>
#include <CImg.h>
#include <dStorm/Output.h>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/engine/TraceReducer.h>
#include <cassert>
#include <Eigen/Core>
#include <vector>
#include "KalmanTrace.h"
#include <dStorm/FilterBuilder.h>

namespace locprec {
    class EmissionTracker : public simparm::Object, public dStorm::Output {
        class _Config;
      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::FilterBuilder<EmissionTracker> Source;

      private:
        ost::Mutex mutex;
        class TracedObject : public KalmanTrace<2> {
            int hope;
          public:
            TracedObject() {}
            TracedObject(const EmissionTracker &papa);

            void add( const dStorm::Localization& l)
                { hope = 2; KalmanTrace<2>::add(l); }
            bool has_lost_hope(int time_difference) const 
                { return (hope - time_difference) < 0; }
            void make_hope() { hope++; }
        };

        typedef data_cpp::List< TracedObject, 16 > List;
        int image_width, image_height, movie_length;

        bool stopped;
        int next_to_track, track_modulo;
        ost::Condition next_to_track_moved;

        List::Allocator allocator;
        List garbage_bin;

        struct TrackingInformation;
        std::vector<TrackingInformation*> tracking;

        Eigen::Matrix<double,2,2> measurement_covar;
        Eigen::Matrix<double,4,4> random_system_dynamics_covar;

        List::iterator getNewEmissionNode(); 

        List::iterator link(const dStorm::Localization &loc,
                            const int imNum);
        void write_positional_image(const TracedObject& position,
                            List::iterator to_write);
        List::iterator start_trace(const dStorm::Localization& loc)
;
        void finalizeImage(int i);

        std::auto_ptr<dStorm::TraceReducer> reducer;
        std::auto_ptr<dStorm::Output> target;

        double maxDist;

      public:
        EmissionTracker( const Config& config,
                         std::auto_ptr<dStorm::Output> output );
        ~EmissionTracker();
        EmissionTracker( const EmissionTracker& );
        EmissionTracker *clone() const
            {throw std::logic_error("Emission tracker is not cloneable.");}
        EmissionTracker& operator=( const EmissionTracker& );

        AdditionalData announceStormSize(const Announcement &) 
;
        void propagate_signal(ProgressSignal s); 
        Result receiveLocalizations(const EngineResult &);

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    class EmissionTracker::_Config : public simparm::Object {
      protected:
        _Config();
        void registerNamedEntries();

      public:
        simparm::UnsignedLongEntry allowBlinking;
        simparm::DoubleEntry expectedDeviation;
        dStorm::TraceReducer::Config reducer;
    };

}

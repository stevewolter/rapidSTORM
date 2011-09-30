#include <dStorm/Image.h>
#include <dStorm/output/Output.h>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/output/TraceReducer.h>
#include <cassert>
#include <Eigen/Core>
#include <vector>
#include "KalmanTrace.h"
#include <dStorm/output/FilterBuilder.h>

#include <dStorm/output/binning/binning.h>
#include <dStorm/UnitEntries/Nanometre.h>
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/ptr_container/ptr_array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional/optional.hpp>

namespace locprec {
    template <typename Type>
    struct address_is_less : public std::binary_function<Type,Type,bool> {
        bool operator()( const Type& a, const Type& b ) const { return &a < &b; };
    };

    class EmissionTracker 
    : public dStorm::output::OutputObject 
    {
        class _Config;
      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::output::FilterBuilder<EmissionTracker> Source;

      private:
        class lowest_mahalanobis_distance;
        class TracedObject : public KalmanTrace<2> {
            int hope;
          public:
            boost::optional<Eigen::Vector2i> cache_position;
            TracedObject(const EmissionTracker &papa);
            ~TracedObject();

            void add( const dStorm::Localization& l)
                { hope = 2; KalmanTrace<2>::add(l); }
            bool has_lost_hope(int time_difference) const 
                { return (hope - time_difference) < 0; }
            void make_hope() { hope++; }
        };

        boost::ptr_array<dStorm::output::binning::Scaled, 2> binners;
        float binner_starts[2];
        typedef dStorm::Image< std::set<TracedObject*>, 2 > Positional;
        Positional positional;

        bool stopped;
        int track_modulo;

        boost::ptr_set< TracedObject, address_is_less<TracedObject> > traced_objects;

        struct TrackingInformation;
        boost::ptr_vector<TrackingInformation> tracking;

        Eigen::Matrix<double,2,2> measurement_covar;
        Eigen::Matrix<double,4,4> random_system_dynamics_covar;

        TracedObject* search_closest_trace(
            const dStorm::Localization &loc, 
            const std::set<TracedObject*>& excluded);
        void update_positional( TracedObject& object );
        void finalizeImage(int i);

        std::auto_ptr<dStorm::output::TraceReducer> reducer;
        std::auto_ptr<dStorm::output::Output> target;

        double maxDist;

      public:
        EmissionTracker( const Config& config,
                         std::auto_ptr<dStorm::output::Output> output );
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
        dStorm::FloatNanometreEntry expectedDeviation;
        dStorm::output::TraceReducer::Config reducer;

        bool determine_output_capabilities
            ( dStorm::output::Capabilities& cap ) 
        { 
            cap.set_intransparency_for_source_data();
            cap.set_cluster_sources( true );
            return true;
        }
    };

}

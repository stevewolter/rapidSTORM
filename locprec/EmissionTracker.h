#include <dStorm/Image.h>
#include <dStorm/output/Output.h>
#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
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
namespace emission_tracker {

    template <typename Type>
    struct address_is_less : public std::binary_function<Type,Type,bool> {
        bool operator()( const Type& a, const Type& b ) const { return &a < &b; };
    };

    class Output 
    : public dStorm::output::OutputObject 
    {
        class _Config;
      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::output::FilterBuilder<Output> Source;

      private:
        class lowest_mahalanobis_distance;
        class TracedObject : public KalmanTrace<2> {
            int hope;
          public:
            boost::optional<Eigen::Vector2i> cache_position;
            TracedObject(const Output &papa);
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

        int track_modulo;

        boost::ptr_set< TracedObject, address_is_less<TracedObject> > traced_objects;

        struct TrackingInformation;
        boost::ptr_vector<TrackingInformation> tracking;

        KalmanMetaInfo<2> kalman_info;

        TracedObject* search_closest_trace(
            const dStorm::Localization &loc, 
            const std::set<TracedObject*>& excluded);
        void update_positional( TracedObject& object );
        void finalizeImage(int i);

        std::auto_ptr<dStorm::output::TraceReducer> reducer;
        std::auto_ptr<dStorm::output::Output> target;

        const double maxDist;

      public:
        Output( const Config& config,
                         std::auto_ptr<dStorm::output::Output> output );
        ~Output();
        Output( const Output& );
        Output *clone() const
            {throw std::logic_error("Emission tracker is not cloneable.");}
        Output& operator=( const Output& );

        AdditionalData announceStormSize(const Announcement &) 
;
        void propagate_signal(ProgressSignal s); 
        Result receiveLocalizations(const EngineResult &);

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    class Output::_Config : public simparm::Object {
      protected:
        _Config();
        void registerNamedEntries();

      public:
        simparm::Entry<unsigned long> allowBlinking;
        dStorm::FloatNanometreEntry expectedDeviation;
        simparm::Entry< boost::units::quantity<KalmanMetaInfo<2>::diffusion_unit> > diffusion;
        simparm::Entry< boost::units::quantity<KalmanMetaInfo<2>::mobility_unit> > mobility;
        dStorm::output::TraceReducer::Config reducer;
        simparm::Entry<float> distance_threshold;

        bool determine_output_capabilities
            ( dStorm::output::Capabilities& cap ) 
        { 
            cap.set_intransparency_for_source_data();
            cap.set_cluster_sources( true );
            return true;
        }
    };

}
}

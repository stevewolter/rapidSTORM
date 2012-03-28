#ifndef LOCPREC_NOISESOURCE_H
#define LOCPREC_NOISESOURCE_H

#include "Fluorophore.h"
#include "FluorophoreDistribution.h"
#include "NoiseGenerator.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/Link.h>
#include <dStorm/engine/InputTraits.h>
#include <gsl/gsl_rng.h>
#include <boost/thread/mutex.hpp>
#include <dStorm/engine/Image_decl.h>

#include <simparm/ChoiceEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/ProgressEntry.hh>
#include <simparm/TreeCallback.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <boost/ptr_container/ptr_list.hpp>
#include <dStorm/traits/optics_config.h>
#include <dStorm/engine/InputTraits.h>

namespace input_simulation {
    class NoiseConfig;

    class NoiseSource 
    : public simparm::Set,
      public dStorm::input::Source< dStorm::engine::ImageStack >
    {
      private:
        unsigned long randomSeed;
        std::auto_ptr< NoiseGenerator<unsigned short> > noiseGenerator;

        typedef dStorm::engine::ImageStack Image;
        typedef dStorm::input::Source<Image> Source;
        boost::shared_ptr< dStorm::input::Traits< Image > > t;
        int imN;
        boost::units::quantity<boost::units::si::time> integration_time;
        typedef boost::ptr_list<Fluorophore> FluorophoreList;
        FluorophoreList fluorophores;
        boost::mutex mutex;

        std::auto_ptr<std::ostream> output;
        void dispatch(dStorm::input::BaseSource::Messages m) { assert( !m.any() ); }

        class iterator;
        simparm::Node& node() { return *this; }

      protected:
        gsl_rng *rng;

      public:
        NoiseSource(const NoiseConfig &config);
        ~NoiseSource();

        dStorm::engine::ImageStack* fetch(int index);

        const boost::ptr_list<Fluorophore>& getFluorophores() const
            { return fluorophores; }

        typedef typename Source::iterator base_iterator;
        base_iterator begin();
        base_iterator end();
        typename Source::TraitsPtr get_traits( typename Source::Wishes );
        typename Source::Capabilities capabilities() const 
            { return typename Source::Capabilities(); }
    };

    struct FluorophoreSetConfig : public simparm::Set {
        void registerNamedEntries();
      public:
        FluorophoreSetConfig(std::string name, std::string desc);
        FluorophoreSetConfig(const FluorophoreSetConfig&);

        FluorophoreConfig fluorophoreConfig;
        simparm::NodeChoiceEntry<FluorophoreDistribution>
            distribution;
        simparm::FileEntry store, recall;
        simparm::Entry<unsigned long> fluorophore_index;

        std::auto_ptr< boost::ptr_list<Fluorophore> > create_fluorophores(
            const dStorm::engine::InputTraits& t,
            gsl_rng*, int imN,
            simparm::ProgressEntry& ) const;
    };

    class NoiseConfig
    : public simparm::Object,
      public dStorm::input::Terminus,
      public simparm::TreeListener
    {
      public:
        typedef std::list< FluorophoreSetConfig* > FluoSets;
        const FluoSets& get_fluorophore_sets() const
            { return fluorophore_sets; }
      private:
        int next_fluo_id;
        FluoSets fluorophore_sets;
        void create_fluo_set();
        void add_fluo_set( std::auto_ptr<FluorophoreSetConfig> );
        std::auto_ptr< dStorm::input::Traits<dStorm::engine::ImageStack> > 
            make_image_size() const;

      protected:
        void operator()( const simparm::Event& );
      public:
        NoiseGeneratorConfig noiseGeneratorConfig;

        simparm::TriggerEntry newSet;
        simparm::Entry<unsigned long> imageNumber;
        simparm::Entry<double> integrationTime;
        simparm::FileEntry saveActivity;
        simparm::Entry<unsigned long> layer_count;
        dStorm::traits::CuboidConfig optics;

        typedef dStorm::engine::ImageStack Image;

        simparm::Node& getNode() { return *this; }

        NoiseConfig();
        NoiseConfig( const NoiseConfig &copy );
        ~NoiseConfig() {}
        void registerNamedEntries();
        void registerNamedEntries( simparm::Node& n ) { n.push_back( *this ); }
        std::string name() const { return getName(); }
        std::string description() const { return getDesc(); }
        void publish_meta_info();

        virtual dStorm::input::Source<Image>* makeSource()
            { return new NoiseSource(*this); }
        virtual NoiseConfig* clone() const 
            { return ( new NoiseConfig(*this) ); }
    };
}

#endif

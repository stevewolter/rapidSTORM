#ifndef LOCPREC_NOISESOURCE_H
#define LOCPREC_NOISESOURCE_H

#include "Fluorophore.h"
#include "FluorophoreDistribution.h"
#include "NoiseGenerator.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/chain/Link.h>
#include <dStorm/ImageTraits.h>
#include <gsl/gsl_rng.h>
#include <dStorm/helpers/thread.h>
#include <dStorm/engine/Image_decl.h>

#include <simparm/ChoiceEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/TreeCallback.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <boost/ptr_container/ptr_list.hpp>
#include <dStorm/traits/optics_config.h>

namespace locprec {
    class NoiseConfig;

    template <typename PixelType> class NoiseSource 
    : public simparm::Set,
      public dStorm::input::Source< dStorm::Image<PixelType,3> >
    {
      private:
        simparm::Entry<unsigned long> &randomSeedEntry;
        std::auto_ptr< NoiseGenerator<PixelType> > noiseGenerator;

        typedef dStorm::Image<PixelType,3> Image;
        typedef dStorm::input::Source<Image> Source;
        typename Image::Size imS;
        int imN;
        boost::units::quantity<boost::units::si::time> integration_time;
        typedef boost::ptr_list<Fluorophore> FluorophoreList;
        FluorophoreList fluorophores;
        ost::Mutex mutex;
        dStorm::traits::Optics<3> optics;

        std::auto_ptr<std::ostream> output;
        void dispatch(dStorm::input::BaseSource::Messages m) { assert( !m.any() ); }

        class iterator;

      protected:
        gsl_rng *rng;

      public:
        NoiseSource(NoiseConfig &config);
        ~NoiseSource();

        dStorm::engine::Image* fetch(int index);

        const boost::ptr_list<Fluorophore>& getFluorophores() const
            { return fluorophores; }
        simparm::Object& getConfig() { return *this; }

        typedef typename Source::iterator base_iterator;
        base_iterator begin();
        base_iterator end();
        typename Source::TraitsPtr get_traits();
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
            dStorm::engine::Image::Size,
            gsl_rng*, int imN, const dStorm::traits::Optics<3>& optics) const;
    };

    class NoiseConfig
    : public simparm::Object,
      public dStorm::input::chain::Terminus,
      public simparm::TreeListener
    {
      public:
        typedef std::list< FluorophoreSetConfig* > FluoSets;
        const FluoSets& get_fluorophore_sets() 
            { return fluorophore_sets; }
      private:
        int next_fluo_id;
        FluoSets fluorophore_sets;
        void create_fluo_set();
        void add_fluo_set( std::auto_ptr<FluorophoreSetConfig> );

        TraitsRef current_traits;
        TraitsRef make_traits() const;

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

        typedef dStorm::engine::Image Image;

        AtEnd context_changed( ContextRef, Link* );
        simparm::Node& getNode() { return *this; }

        NoiseConfig();
        NoiseConfig( const NoiseConfig &copy );
        ~NoiseConfig() {}
        void registerNamedEntries();

        virtual dStorm::input::Source<Image>* makeSource()
            { return new NoiseSource<Image::Pixel>(*this); }
        virtual NoiseConfig* clone() const 
            { return ( new NoiseConfig(*this) ); }
    };
}

#endif

#ifndef LOCPREC_NOISESOURCE_H
#define LOCPREC_NOISESOURCE_H

#include "input_simulation/Fluorophore.h"
#include "input_simulation/FluorophoreDistribution.h"
#include "input_simulation/NoiseGenerator.h"
#include "input/Source.h"
#include "input/Link.h"
#include "engine/InputTraits.h"
#include <gsl/gsl_rng.h>
#include <boost/thread/mutex.hpp>
#include "engine/Image_decl.h"
#include "units/microlength.h"

#include "simparm/ChoiceEntry.h"
#include "simparm/ProgressEntry.h"
#include <boost/ptr_container/ptr_list.hpp>
#include "traits/optics_config.h"
#include "engine/InputTraits.h"

namespace input_simulation {
    class NoiseConfig;

    struct FluorophoreSetConfig {
        simparm::Group name_object;
      public:
        FluorophoreSetConfig(std::string name, std::string desc);

        FluorophoreConfig fluorophoreConfig;
        simparm::ManagedChoiceEntry<FluorophoreDistribution>
            distribution;
        simparm::FileEntry store, recall;
        simparm::Entry<unsigned long> fluorophore_index;

        std::auto_ptr< boost::ptr_list<Fluorophore> > create_fluorophores(
            const dStorm::engine::InputTraits& t,
            gsl_rng*, const NoiseConfig&,
            simparm::ProgressEntry& ) const;

        void attach_ui( simparm::NodeHandle );
    };

    class NoiseConfig
    : public dStorm::input::Terminus
    {
      public:
        typedef boost::ptr_list< FluorophoreSetConfig > FluoSets;
        const FluoSets& get_fluorophore_sets() const
            { return fluorophore_sets; }
      private:
        simparm::Object name_object;
        simparm::NodeHandle current_ui;
        FluoSets fluorophore_sets;
        simparm::BaseAttribute::ConnectionStore listening[2];

        void commit_fluo_set_count();
        std::auto_ptr< dStorm::input::Traits<dStorm::engine::ImageStack> > 
            make_image_size() const;

        void optics_changed();
        void notice_layer_count();

      public:
        NoiseGeneratorConfig noiseGeneratorConfig;

        simparm::Entry<unsigned long> fluo_set_count;
        simparm::Entry<unsigned long> imageNumber;
        simparm::Entry< quantity<si::microlength> > sample_depth;
        simparm::Entry<double> integrationTime;
        simparm::FileEntry saveActivity;
        simparm::Entry<unsigned long> layer_count;
        dStorm::traits::MultiPlaneConfig optics;

        typedef dStorm::engine::ImageStack Image;

        NoiseConfig();
        NoiseConfig( const NoiseConfig &copy );
        ~NoiseConfig() {}
        void registerNamedEntries( simparm::NodeHandle n );
        std::string name() const { return name_object.getName(); }
        std::string description() const { return name_object.getDesc(); }
        void publish_meta_info();

        dStorm::input::Source<Image>* makeSource();
        NoiseConfig* clone() const 
            { return ( new NoiseConfig(*this) ); }
    };
    class NoiseSource 
    : public dStorm::input::Source< dStorm::engine::ImageStack >
    {
      private:
        unsigned long randomSeed;
        std::auto_ptr< NoiseGenerator<unsigned short> > noiseGenerator;
        boost::ptr_list< FluorophoreSetConfig > fluorophore_configs;
        const NoiseConfig noise_config;

        typedef dStorm::engine::ImageStack Image;
        typedef dStorm::input::Source<Image> Source;
        boost::shared_ptr< dStorm::input::Traits< Image > > t;
        int current_image;
        int imN;
        boost::units::quantity<boost::units::si::time> integration_time;
        typedef boost::ptr_list<Fluorophore> FluorophoreList;
        FluorophoreList fluorophores;
        boost::mutex mutex;

        simparm::Group name_object;
        simparm::NodeHandle current_ui;

        std::auto_ptr<std::ostream> output;
        void dispatch(dStorm::input::BaseSource::Messages m) { assert( !m.any() ); }

        void attach_ui_( simparm::NodeHandle n ) { current_ui = name_object.attach_ui(n); }

      protected:
        gsl_rng *rng;

      public:
        NoiseSource(const NoiseConfig &config);
        ~NoiseSource();

        const boost::ptr_list<Fluorophore>& getFluorophores() const
            { return fluorophores; }

        typename Source::TraitsPtr get_traits();
        void set_thread_count(int num_threads) OVERRIDE {}
        bool GetNext(int thread, dStorm::engine::ImageStack* output) OVERRIDE;
    };

}

#endif

#define BOOST_DISABLE_ASSERTS
#define LOCPREC_NOISESOURCE_CPP

#include "debug.h"
#include "NoiseSource.h"
#include <time.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>
#include <cassert>
#include <dStorm/engine/Image.h>
#include <fstream>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <dStorm/Image_impl.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/engine/InputTraits.h>
#include <boost/units/Eigen/Array>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <dStorm/threed_info/Config.h>
#include <fstream>

#include "FluorophoreDistributions.h"

using namespace boost::units;

namespace input_simulation {

void FluorophoreSetConfig::attach_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle h = name_object.attach_ui( at );
    fluorophoreConfig.attach_ui( h );
    distribution.attach_ui( h );
    store.attach_ui( h );
    recall.attach_ui( h );
    fluorophore_index.attach_ui( h );
}

void NoiseConfig::registerNamedEntries( simparm::NodeHandle n ) {
    listening[0] = fluo_set_count.value.notify_on_value_change( 
        boost::bind( &NoiseConfig::commit_fluo_set_count, this ) );
    listening[1] = layer_count.value.notify_on_value_change( 
        boost::bind( &NoiseConfig::notice_layer_count, this ) );

    simparm::NodeHandle r = name_object.attach_ui( n );
    noiseGeneratorConfig.attach_ui( r );
    layer_count.attach_ui( r );
    optics.attach_ui( r );
    fluo_set_count.attach_ui( r );
    imageNumber.attach_ui( r );
    sample_depth.attach_ui( r );
    integrationTime.attach_ui( r );
    saveActivity.attach_ui( r );

    current_ui = r;
    for ( FluoSets::iterator i = fluorophore_sets.begin(); i != fluorophore_sets.end(); ++i )
        i->attach_ui( current_ui );
}

#define NAME "Generated"
#define DESC "Randomly generated values"

FluorophoreSetConfig::FluorophoreSetConfig(std::string name, std::string desc)
: name_object(name, desc),
  distribution( "FluorophoreDistribution", "Pattern to "
                           "distribute simulated fluorophores with" ),
  store("StoreFluorophores", "Store fluorophore positions and PSFs", "" ),
  recall("RecallFluorophores", "Recall fluorophore positions and PSFs", "" ),
  fluorophore_index("FluorophoreIndex", "Fluorophore ident for transmission", 0)
{ 
    distribution.addChoice( new FluorophoreDistributions::Random());
    distribution.addChoice( FluorophoreDistributions::make_lattice());
    distribution.addChoice( FluorophoreDistributions::make_lines());
}

NoiseConfig::NoiseConfig()
: name_object(NAME, DESC),
  fluo_set_count("FluoSetCount", "Number of fluorophore sets", 1),
  imageNumber("ImageNumber", "Number of source images to generate", 10000),
  sample_depth("SampleDepth", "Depth of virtual sample", 1 * si::micrometer),
  integrationTime("IntegrationTime", "Integration time for one image", 0.1),
  saveActivity( "SaveActivity", "Save fluorophore activity information to file", "" ),
  layer_count( "LayerCount", "Number of layers to generate", 1 ),
  optics( dStorm::traits::PlaneConfig::InputSimulation )
{
    commit_fluo_set_count();
    optics.notify_on_any_change( boost::bind( &NoiseConfig::publish_meta_info, this ) );
}

NoiseConfig::NoiseConfig( const NoiseConfig & cp )
: dStorm::input::Terminus(cp),
  name_object(cp.name_object),
  fluorophore_sets(cp.fluorophore_sets),
  noiseGeneratorConfig(cp.noiseGeneratorConfig),
  fluo_set_count(cp.fluo_set_count),
  imageNumber(cp.imageNumber),
  sample_depth(cp.sample_depth),
  integrationTime(cp.integrationTime),
  saveActivity(cp.saveActivity),
  layer_count(cp.layer_count),
  optics(cp.optics)
{
    optics.notify_on_any_change( boost::bind( &NoiseConfig::publish_meta_info, this ) );
}

void NoiseConfig::notice_layer_count() {
    std::auto_ptr< dStorm::input::Traits<Image> > image
        = make_image_size();
    optics.set_context( *image );
    publish_meta_info();
}

void NoiseConfig::commit_fluo_set_count()
{
    size_t count = fluo_set_count();
    while ( count > fluorophore_sets.size() ) {
        std::string id = boost::lexical_cast<std::string>( fluorophore_sets.size() + 1 );
        fluorophore_sets.push_back
            ( new FluorophoreSetConfig("FluorophoreSet" + id,
                                    "Fluorpohore set " + id) );
        if ( current_ui ) fluorophore_sets.back().attach_ui( current_ui );
    }
    while ( count < fluorophore_sets.size() ) {
        fluorophore_sets.pop_back();
    }
}

std::auto_ptr< boost::ptr_list<Fluorophore> >
FluorophoreSetConfig::create_fluorophores(
    const dStorm::engine::InputTraits& t,
    gsl_rng *rng,
    const NoiseConfig& config,
    simparm::ProgressEntry& progress ) const
{
    std::auto_ptr< boost::ptr_list<Fluorophore> > fluorophores
        ( new boost::ptr_list<Fluorophore>() );
    if ( recall ) {
        std::ifstream in( recall().c_str() );
        int number;
        in >> number;

        for (int i = 0; i < number; i++)
            fluorophores->push_back( new Fluorophore( in,
                fluorophoreConfig ) );
    } else {
        const FluorophoreDistribution& distribution = this->distribution();
        dStorm::samplepos size = t.size_in_sample_space().second;
        size.z() = quantity<si::length>(config.sample_depth());
        FluorophoreDistribution::Positions positions = 
            distribution.fluorophore_positions( size, rng);

        int bins = 100;
        dStorm::Image<Fluorophore*,2>::Size sz;
        sz.x() = sz.y() = (bins+1) * camera::pixel;
        dStorm::Image<Fluorophore*,2> cache(sz);
        cache.fill(0);

        int target_fluorophore_count = positions.size();
        while ( ! positions.empty() ) {
            progress.setValue( double(fluorophores->size()) / target_fluorophore_count );
            fluorophores->push_back( new Fluorophore(positions.front(), config.imageNumber(), fluorophoreConfig, t, fluorophore_index()) );
            positions.pop();
        }
    }

    if ( store ) {
        std::ofstream out( store().c_str() );
        out << fluorophores->size() << "\n";
        boost::ptr_list<Fluorophore>::const_iterator i;
        for ( i = fluorophores->begin(); i != fluorophores->end(); ++i )
            out << *i << "\n";
    }

    return fluorophores;
}

NoiseSource::NoiseSource( const NoiseConfig &config )
: randomSeed(config.noiseGeneratorConfig.random_seed()),
  fluorophore_configs( config.get_fluorophore_sets() ),
  noise_config( config ),
  t( new dStorm::engine::InputTraits() ),
  name_object("NoiseSource", "Noise source status")
{

    DEBUG("Just made traits for noise source");
    rng = gsl_rng_alloc(gsl_rng_mt19937);
    DEBUG("Using random seed " << randomSeed);
    gsl_rng_set(rng, randomSeed );
    noiseGenerator = 
        NoiseGenerator<unsigned short>::factory(config.noiseGeneratorConfig, rng);

    for (size_t p = 0; p < config.layer_count(); ++p) {
        dStorm::image::MetaInfo<2> size;
        size.size.x() = config.noiseGeneratorConfig.width() * camera::pixel;
        size.size.y() = config.noiseGeneratorConfig.height() * camera::pixel;
        t->push_back( size, dStorm::traits::Optics() );
    }
    config.optics.write_traits( *t );
    for (int p = 0; p < t->plane_count(); ++p) {
        t->plane(p).create_projection();
    }
    imN = config.imageNumber();
    integration_time = config.integrationTime() * si::seconds;

    if ( config.saveActivity )
        output.reset( new std::ofstream
            ( config.saveActivity().c_str() ));
}

NoiseSource::~NoiseSource()

{
    gsl_rng_free(rng);
}

dStorm::engine::ImageStack* NoiseSource::fetch( int imNum )
{
    boost::lock_guard<boost::mutex> lock(mutex);
    std::auto_ptr<dStorm::engine::ImageStack>
        result(new dStorm::engine::ImageStack(imNum * camera::frame));

    for ( int p = 0; p < t->plane_count(); ++p ) {
        dStorm::engine::Image2D i( t->image(p).size );
        noiseGenerator->pixelNoise(i.ptr(), i.size_in_pixels());
        result->push_back( i );
    }

    /* Then add the fluorophores. */
    DEBUG("Making glare for " << fluorophores.size() << " fluorophores");
    if ( output.get() ) *output << imNum;
    int index = 0;
    BOOST_FOREACH( Fluorophore& fl, fluorophores )  {
        int photons =
            fl.glareInImage(rng, *result, imNum, integration_time);
        if ( photons > 0 && output.get() ) *output << ", " << index << " " << photons;
        ++index;
    }
    if ( output.get() ) *output << "\n";

    return result.release();
}

class NoiseSource::iterator
: public boost::iterator_facade<iterator,Image,std::input_iterator_tag>
{
    friend class boost::iterator_core_access;
    NoiseSource* const src;
    mutable boost::shared_ptr<Image> img;
    int image_number;
    
    Image& dereference() const { 
        if ( img.get() == NULL ) {
            img.reset( src->fetch(image_number) ); 
        }
        return *img; 
    }
    void increment() { img.reset(); image_number++; }
    bool equal(const iterator& i) const { return image_number == i.image_number; }

  public:
    iterator() : src(NULL), image_number(0) {}
    iterator(NoiseSource& ns, int im) : src(&ns), image_number(im) {}
};

NoiseSource::base_iterator 
NoiseSource::begin() {
    return base_iterator( iterator(*this, 0) );
}

NoiseSource::base_iterator 
NoiseSource::end() {
    return base_iterator( iterator(*this, imN) );
}

NoiseSource::Source::TraitsPtr
NoiseSource::get_traits( typename Source::Wishes ) {
    simparm::ProgressEntry progress("FluorophoreProgress", "Fluorophore generation progress");
    simparm::NodeHandle ui = progress.attach_ui( current_ui );
    for ( boost::ptr_list< FluorophoreSetConfig >::const_iterator
            i = fluorophore_configs.begin(); i != fluorophore_configs.end(); ++i)
    {
        std::auto_ptr<FluorophoreList> l = 
            i->create_fluorophores( *t, rng, noise_config, progress );
        fluorophores.transfer( fluorophores.end(), *l );
    }

    return t;
}

std::auto_ptr< dStorm::input::Traits<dStorm::engine::ImageStack> >
NoiseConfig::make_image_size() const
{
    typedef dStorm::input::Traits<dStorm::engine::ImageStack> Traits;

    std::auto_ptr< Traits > rv( new Traits() );
    rv->image_number().range().first = 0 * camera::frame;
    rv->image_number().range().second = dStorm::traits::ImageNumber::ValueType
        ::from_value( (imageNumber() - 1) );
    for (size_t p = 0; p < layer_count(); ++p) {
        dStorm::image::MetaInfo<2> meta_info;
        meta_info.size.x() = noiseGeneratorConfig.width() * camera::pixel;
        meta_info.size.y() = noiseGeneratorConfig.height() * camera::pixel;
        rv->push_back( meta_info, dStorm::traits::Optics() );
    }
    return rv;
}

void NoiseConfig::publish_meta_info() {
    dStorm::input::MetaInfo::Ptr t( new dStorm::input::MetaInfo() );
    t->set_traits( make_image_size().release() );
    update_current_meta_info( t );
}

dStorm::input::Source<NoiseConfig::Image>* NoiseConfig::makeSource() { return new NoiseSource(*this); }

}

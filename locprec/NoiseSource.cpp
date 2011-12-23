#define BOOST_DISABLE_ASSERTS
#define LOCPREC_NOISESOURCE_CPP

#include <dStorm/debug.h>
#include "NoiseSource.h"
#include <time.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>
#include "foreach.h"
#include <cassert>
#include <dStorm/helpers/thread.h>
#include <dStorm/engine/Image.h>
#include <fstream>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <dStorm/Image_impl.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/MetaInfo.h>
#include <boost/units/Eigen/Array>
#include <boost/iterator/iterator_facade.hpp>

#include "FluorophoreDistributions.h"

using namespace boost::units;

namespace locprec {

void FluorophoreSetConfig::registerNamedEntries() {
    this->push_back( fluorophoreConfig );
    this->push_back( distribution );
    this->push_back( store );
    this->push_back( recall );
    this->push_back( fluorophore_index );
}

void NoiseConfig::registerNamedEntries() {
    this->receive_changes_from( newSet.value );
    this->receive_changes_from( layer_count.value );
    receive_changes_from_subtree( optics ); 

    this->push_back( noiseGeneratorConfig );
    this->push_back( layer_count );
    optics.registerNamedEntries();
    this->push_back( optics );
    this->push_back( newSet );
    this->push_back( imageNumber );
    this->push_back( integrationTime );
    this->push_back( saveActivity );
}

#define NAME "Generated"
#define DESC "Randomly generated values"

FluorophoreSetConfig::FluorophoreSetConfig(std::string name, std::string desc)
: simparm::Set(name, desc),
  distribution( "FluorophoreDistribution", "Pattern to "
                           "distribute simulated fluorophores with" ),
  store("StoreFluorophores", "Store fluorophore positions and PSFs" ),
  recall("RecallFluorophores", "Recall fluorophore positions and PSFs" ),
  fluorophore_index("FluorophoreIndex", "Fluorophore ident for transmission", 0)
{ 
    distribution.addChoice( new FluorophoreDistributions::Random());
    distribution.addChoice( new FluorophoreDistributions::Lattice());
    distribution.addChoice( new FluorophoreDistributions::Lines());

    registerNamedEntries();
}

FluorophoreSetConfig::FluorophoreSetConfig(const FluorophoreSetConfig& o)
: simparm::Set(o),
  fluorophoreConfig(o.fluorophoreConfig),
  distribution(o.distribution),
  store(o.store),
  recall(o.recall),
  fluorophore_index(o.fluorophore_index)
{
    registerNamedEntries();
}

NoiseConfig::NoiseConfig()
: simparm::Object(NAME, DESC),
  simparm::TreeListener(),
  next_fluo_id(1),
  newSet("NewFluorophoreSet", "Add fluorophore set"),
  imageNumber("ImageNumber", "Number of source images to generate", 10000),
  integrationTime("IntegrationTime", "Integration time for one image", 0.1),
  saveActivity( "SaveActivity", "Save fluorophore activity information to file" ),
  layer_count( "LayerCount", "Number of layers to generate", 1 )
{
    create_fluo_set();
    registerNamedEntries();
    optics.set_number_of_fluorophores(2);
}

NoiseConfig::NoiseConfig( const NoiseConfig & cp )
: simparm::Object(cp),
  dStorm::input::Terminus(cp),
  simparm::TreeListener(),
  next_fluo_id(cp.next_fluo_id),
  noiseGeneratorConfig(cp.noiseGeneratorConfig),
  newSet(cp.newSet),
  imageNumber(cp.imageNumber),
  integrationTime(cp.integrationTime),
  saveActivity(cp.saveActivity),
  layer_count(cp.layer_count),
  optics(cp.optics)
{
    for ( FluoSets::const_iterator i = cp.fluorophore_sets.begin();
                                   i != cp.fluorophore_sets.end(); ++i)
        add_fluo_set( std::auto_ptr<FluorophoreSetConfig>(
            new FluorophoreSetConfig(**i) ) );
    registerNamedEntries();
}

void NoiseConfig::operator()( const simparm::Event& e)
{
    if ( newSet.triggered() ) {
        create_fluo_set();
        newSet.untrigger();
    } else if ( e.cause == simparm::Event::ValueChanged ) {
        if ( &e.source == &layer_count.value )
            optics.set_number_of_planes( layer_count() );
        publish_meta_info();
    } else 
	TreeListener::add_new_children(e);
}

void NoiseConfig::create_fluo_set()
{
    std::stringstream id;
    id << next_fluo_id++;
    add_fluo_set( std::auto_ptr<FluorophoreSetConfig>
        ( new FluorophoreSetConfig("FluorophoreSet" + id.str(),
                                   "Fluorpohore set " + id.str()) ));
}

void NoiseConfig::add_fluo_set( std::auto_ptr<FluorophoreSetConfig> s )
{
    fluorophore_sets.push_back( s.get() );
    this->simparm::Node::push_back( std::auto_ptr<simparm::Node>(s) );
}

std::auto_ptr< boost::ptr_list<Fluorophore> >
FluorophoreSetConfig::create_fluorophores(
    dStorm::engine::Image::Size imS,
    gsl_rng *rng,
    int imN, const dStorm::traits::Optics<3>& optics) const
{
    std::auto_ptr< boost::ptr_list<Fluorophore> > fluorophores
        ( new boost::ptr_list<Fluorophore>() );
    if ( recall ) {
        simparm::FileEntry opener( recall );
        std::istream& in = opener.get_input_stream();
        int number;
        in >> number;

        for (int i = 0; i < number; i++)
            fluorophores->push_back( new Fluorophore( in,
                fluorophoreConfig ) );
    } else {
        FluorophoreDistribution::Positions positions;
        const FluorophoreDistribution& distribution = this->distribution.value();
        positions = distribution.fluorophore_positions(
            optics.size_in_sample_space( imS.head<2>().cast< dStorm::traits::Optics<2>::SubpixelImagePosition::Scalar >() ), rng);

        int bins = 100;
        dStorm::Image<Fluorophore*,2>::Size sz;
        sz.x() = sz.y() = (bins+1) * camera::pixel;
        dStorm::Image<Fluorophore*,2> cache(sz);
        cache.fill(0);

        while ( ! positions.empty() ) {
            fluorophores->push_back( new Fluorophore(positions.front(), imN, fluorophoreConfig, optics, fluorophore_index()) );
            positions.pop();
        }
    }

    if ( store ) {
        simparm::FileEntry opener( store );
        std::ostream& out = opener.get_output_stream();
        out << fluorophores->size() << "\n";
        boost::ptr_list<Fluorophore>::const_iterator i;
        for ( i = fluorophores->begin(); i != fluorophores->end(); ++i )
            out << *i << "\n";
    }

    return fluorophores;
}

template <typename Pixel>
NoiseSource<Pixel>::NoiseSource( NoiseConfig &config )

: simparm::Set("NoiseSource", "Noise source status"),
  randomSeedEntry(config.noiseGeneratorConfig.random_seed),
  optics( config.optics.make_traits() )
{
    DEBUG("Just made traits for noise source");
    rng = gsl_rng_alloc(gsl_rng_mt19937);
    DEBUG("Using random seed " << randomSeedEntry());
    gsl_rng_set(rng, randomSeedEntry() );
    noiseGenerator = 
        NoiseGenerator<Pixel>::factory(config.noiseGeneratorConfig, rng);

    this->imS.fill(1 * camera::pixel);
    this->imS.x() = config.noiseGeneratorConfig.width() * camera::pixel;
    this->imS.y() = config.noiseGeneratorConfig.height() * camera::pixel;
    this->imS.z() = config.optics.number_of_planes() * camera::pixel;
    imN = config.imageNumber();
    integration_time = config.integrationTime() * si::seconds;

    if ( config.saveActivity )
        output.reset( new std::ofstream
            ( config.saveActivity().c_str() ));

    for ( NoiseConfig::FluoSets::const_iterator
            i = config.get_fluorophore_sets().begin();
            i != config.get_fluorophore_sets().end(); ++i)
    {
        std::auto_ptr<FluorophoreList> l = 
            (*i)->create_fluorophores( imS, rng, imN, optics );
        fluorophores.transfer( fluorophores.end(), *l );
    }

}

template <typename Pixel>
NoiseSource<Pixel>::~NoiseSource()

{
    /* The last value of our random number generator is the next
     * seed. */
    randomSeedEntry = gsl_rng_get(rng);
    gsl_rng_free(rng);
}

template <typename Pixel>
dStorm::engine::Image* NoiseSource<Pixel>::fetch( int imNum )
{
    ost::MutexLock lock(mutex);
    std::auto_ptr<dStorm::engine::Image>
        result(new dStorm::engine::Image(this->imS, imNum * camera::frame));

    noiseGenerator->pixelNoise(result->ptr(), result->size_in_pixels());

    /* Then add the fluorophores. */
    DEBUG("Making glare for " << fluorophores.size() << " fluorophores");
    if ( output.get() ) *output << imNum;
    int index = 0;
    foreach( fl, FluorophoreList, fluorophores )  {
        int photons =
            fl->glareInImage(rng, *result, imNum, integration_time);
        if ( photons > 0 && output.get() ) *output << ", " << index << " " << photons;
        ++index;
    }
    if ( output.get() ) *output << "\n";

    return result.release();
}


template <typename Pixel>
class NoiseSource<Pixel>::iterator
: public boost::iterator_facade<iterator,Image,std::input_iterator_tag>
{
    friend class boost::iterator_core_access;
    NoiseSource<Pixel>* const src;
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
    iterator(NoiseSource<Pixel>& ns, int im) : src(&ns), image_number(im) {}
};

template <typename Pixel>
typename NoiseSource<Pixel>::base_iterator 
NoiseSource<Pixel>::begin() {
    return base_iterator( iterator(*this, 0) );
}

template <typename Pixel>
typename NoiseSource<Pixel>::base_iterator 
NoiseSource<Pixel>::end() {
    return base_iterator( iterator(*this, imN) );
}

template <typename Pixel>
typename NoiseSource<Pixel>::Source::TraitsPtr
NoiseSource<Pixel>::get_traits( typename Source::Wishes ) {
    typename Source::TraitsPtr rv( new dStorm::input::Traits< dStorm::Image<Pixel,3> >() );
    static_cast< dStorm::traits::Optics<3>& >(*rv) = optics;
    rv->size = imS;
    rv->image_number().range().first = 0 * camera::frame;
    rv->image_number().range().second = (imN - 1) * camera::frame;
    return rv;
}

void NoiseConfig::publish_meta_info() {
    typedef dStorm::input::Traits<Image> Traits;

    boost::shared_ptr< Traits > rv( new Traits() );
    rv->size.x() = noiseGeneratorConfig.width() * camera::pixel;
    rv->size.y() = noiseGeneratorConfig.height() * camera::pixel;
    rv->image_number().range().first = 0 * camera::frame;
    rv->image_number().range().second = dStorm::traits::ImageNumber::ValueType
        ::from_value( (imageNumber() - 1) );
    static_cast< dStorm::traits::Optics<3>& >(*rv) = optics.make_traits();
    rv->fluorophores[1].wavelength = rv->fluorophores[0].wavelength;

    dStorm::input::MetaInfo::Ptr t( new dStorm::input::MetaInfo() );
    t->set_traits( rv );
    update_current_meta_info( t );
}

}

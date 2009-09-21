#define LOCPREC_NOISESOURCE_CPP
#undef NDEBUG

#include "NoiseSource.h"
#include <time.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>
#include "foreach.h"
#include <cassert>
#include <cc++/thread.h>
#include <dStorm/engine/Image.h>
#include <CImg.h>
#include <fstream>

#include "FluorophoreDistributions.h"

namespace locprec {

template <typename Pixel>
void NoiseConfig<Pixel>::registerNamedEntries() {
    this->push_back( noiseGeneratorConfig );
    this->register_entry(&imageNumber);
    this->register_entry(&distribution);
    this->push_back( fluorophoreConfig );
    this->push_back( saveActivity );
    this->push_back( store );
    this->push_back( recall );
}

#define NAME "Generated"
#define DESC "Randomly generated values"

template <typename Pixel>
NoiseConfig<Pixel>::NoiseConfig( CImgBuffer::Config & )

: CImgBuffer::InputConfig< CImgBuffer::Image<Pixel> >(NAME, DESC),
  imageNumber("ImageNumber", "Number of source images to generate", 10000),
  distribution( "FluorophoreDistribution", "Pattern to "
                           "distribute simulated fluorophores with" ),
  saveActivity( "SaveActivity", "Save fluorophore activity information to file" ),
  store("StoreFluorophores", "Store fluorophore positions and PSFs" ),
  recall("RecallFluorophores", "Recall fluorophore positions and PSFs" )
{
    distribution.addChoice( new FluorophoreDistributions::Random());
    distribution.addChoice( new FluorophoreDistributions::Lattice());
    distribution.addChoice( new FluorophoreDistributions::Lines());

    registerNamedEntries();
}

template <typename Pixel>
NoiseConfig<Pixel>::NoiseConfig( const NoiseConfig<Pixel> & cp,
    CImgBuffer::Config& )

: CImgBuffer::InputConfig< CImgBuffer::Image<Pixel> >(cp),
  fluorophoreConfig(cp.fluorophoreConfig),
  noiseGeneratorConfig(cp.noiseGeneratorConfig),
  imageNumber(cp.imageNumber),
  distribution(cp.distribution),
  saveActivity(cp.saveActivity),
  store(cp.store),
  recall(cp.recall)
{
    registerNamedEntries();
}

template <typename Pixel>
NoiseSource<Pixel>::NoiseSource( NoiseConfig<Pixel> &config )

: CImgBuffer::Source< cimg_library::CImg<Pixel> >( BaseSource::Pullable ),
  simparm::Set("NoiseSource", "Noise source status"),
  randomSeedEntry(config.noiseGeneratorConfig.random_seed)
{
    rng = gsl_rng_alloc(gsl_rng_mt19937);
    STATUS("Using random seed " << randomSeedEntry());
    gsl_rng_set(rng, randomSeedEntry() );
    noiseGenerator = 
        NoiseGenerator<Pixel>::factory(config.noiseGeneratorConfig, rng);

    imW = config.noiseGeneratorConfig.width();
    imH = config.noiseGeneratorConfig.height();
    imN = config.imageNumber();

    if ( config.saveActivity )
        output.reset( new std::ofstream
            ( config.saveActivity().c_str() ));

  if ( config.recall ) {
    std::istream& in = config.recall.get_input_stream();
    int number;
    in >> number;

    for (int i = 0; i < number; i++)
        fluorophores.push_back( new Fluorophore( in,
            config.fluorophoreConfig ) );
  } else {
    FluorophoreDistribution::Positions positions;
    FluorophoreDistribution& distribution = config.distribution.value();
    positions = distribution.fluorophore_positions(
                    FluorophoreDistribution::Size(imW, imH), rng
                );

    int bins = 100;
    cimg_library::CImg<Fluorophore*> cache(bins+1, bins+1);
    cache.fill(0);

    while ( ! positions.empty() ) {
        Fluorophore *fresh;
        Fluorophore::Position p = positions.front(),
                offset = p - p.cast<int>().cast<double>();
        int cx = round(offset[0]*bins), cy = round(offset[1]*bins);
        assert( cx >= 0 && cx <= bins );
        assert( cy >= 0 && cy <= bins );
        Fluorophore*& cell = cache( cx, cy );
        if ( cell != NULL ) {
            fresh = new Fluorophore(*cell);
            fresh->recenter( p );
        } else {
            fresh = new Fluorophore(positions.front(), imN,
                                    config.fluorophoreConfig);
        }
        cell = fresh;
        fluorophores.push_back( fresh );
        positions.pop();
    }
  }

    if ( config.store ) {
        std::ostream& out = config.store.get_output_stream();
        out << fluorophores.size() << "\n";
        std::list<Fluorophore*>::const_iterator i;
        for ( i = fluorophores.begin(); i != fluorophores.end(); ++i )
            out << **i << "\n";
    }
}

template <typename Pixel>
NoiseSource<Pixel>::~NoiseSource()

{
    /* The last value of our random number generator is the next
     * seed. */
    randomSeedEntry = gsl_rng_get(rng);
    gsl_rng_free(rng);
    foreach( fl, std::list<Fluorophore*>, fluorophores ) 
        delete *fl;
}

template <typename Pixel>
dStorm::Image* NoiseSource<Pixel>::fetch( int imNum )

{
    ost::MutexLock lock(mutex);
    dStorm::Image* result = (new dStorm::Image(imW, imH));

    noiseGenerator->pixelNoise(result->data, result->size());

    /* Then add the fluorophores. */
    PROGRESS("Making glare for " << fluorophores.size() << 
             " fluorophores");
    if ( output.get() ) *output << imNum << " ";
    foreach( fl, std::list<Fluorophore*>, fluorophores )  {
        int photons =
            (*fl)->glareInImage(rng, *result, imNum, 0.1);
        if ( output.get() ) *output << photons << " ";
    }
    if ( output.get() ) *output << "\n";

    return result;
}

template class NoiseConfig<dStorm::StormPixel>;
template class NoiseSource<dStorm::StormPixel>;

}

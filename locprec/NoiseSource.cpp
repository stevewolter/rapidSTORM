#define LOCPREC_NOISESOURCE_CPP

#include "NoiseSource.h"
#include <time.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>
#include "foreach.h"
#include <cassert>
#include <cc++/thread.h>

namespace locprec {

template <typename Pixel>
void NoiseMethod<Pixel>::registerMethod() throw()
{
    registerInputMethod(new NoiseMethod<Pixel>());
}

template <typename Pixel>
void NoiseConfig<Pixel>::registerNamedEntries() throw() {
    this->push_back( noiseGeneratorConfig );
    this->register_entry(&imageNumber);
    this->register_entry(&fluorophoreNumber);
    this->register_entry(&fluorophoreDistribution);
    this->push_back( fluorophoreConfig );
}

#define NAME "Generated"
#define DESC "Randomly generated values"

template <typename Pixel>
NoiseConfig<Pixel>::NoiseConfig( CImgBuffer::Config<Image> & )
    throw()
: simparm::ConfigObject(NAME, DESC),
  CImgBuffer::InputConfig< CImgBuffer::Image<Pixel> >(),
  imageNumber("ImageNumber", "Number of source images to generate", 10000),
  fluorophoreNumber("FluorophoreNumber", "Number of "
                    "fluorophore in virtual sample", 100),
  fluorophoreDistribution( "FluorophoreDistribution", "Pattern to "
                           "distribute simulated fluorophores with" )
{
    fluorophoreDistribution.addChoice("lattice", 
                                      "Rectangular lattice");
    fluorophoreDistribution.addChoice("random", "Randomly");

    registerNamedEntries();
}

template <typename Pixel>
NoiseConfig<Pixel>::NoiseConfig( const NoiseConfig<Pixel> & cp,
    CImgBuffer::Config<Image>& 
)
    throw()
: simparm::ConfigObject(cp),
  CImgBuffer::InputConfig< CImgBuffer::Image<Pixel> >(cp),
  fluorophoreConfig(cp.fluorophoreConfig),
  noiseGeneratorConfig(cp.noiseGeneratorConfig),
  imageNumber(cp.imageNumber),
  fluorophoreNumber(cp.fluorophoreNumber),
  fluorophoreDistribution(cp.fluorophoreDistribution)
{
    registerNamedEntries();
}

template <typename Pixel>
NoiseSource<Pixel>::NoiseSource( NoiseConfig<Pixel> &config )
    throw()
: simparm::ConfigObject("NoiseSource", "Noise source status"),
  randomSeedEntry(config.noiseGeneratorConfig.random_seed)
{
    rng = gsl_rng_alloc(gsl_rng_mt19937);
    STATUS("Using random seed " << config.random_seed());
    gsl_rng_set(rng, randomSeedEntry() );
    noiseGenerator = 
        NoiseGenerator<Pixel>::factory(config.noiseGeneratorConfig, rng);

    imW = config.noiseGeneratorConfig.width();
    imH = config.noiseGeneratorConfig.height();
    imN = config.imageNumber();

    if (config.fluorophoreNumber() == 0) return;

    STATUS("Making " << config.fluorophoreNumber() << " fluorophores");
    if (config.fluorophoreDistribution == "random") {
        for (unsigned int i = 0; i < config.fluorophoreNumber(); i++) {
            double x = gsl_rng_uniform(rng) * imW;
            double y = gsl_rng_uniform(rng) * imH;
            fluorophores.push_back(
                new Fluorophore(x, y, imN, config.fluorophoreConfig));
        }
    } else if (config.fluorophoreDistribution == "lattice") {
        int border = 4;
        int rows = int(floor(sqrt(config.fluorophoreNumber())));
        int maxY = (config.fluorophoreNumber() / rows)+1;
        for (unsigned int i = 0; i < config.fluorophoreNumber(); i++) {
            double x = border + (double(i % rows) / (rows)) * 
                        (imW-2*border);
            double y = border + (double(i / rows) / maxY) * 
                            (imH-2*border);
            fluorophores.push_back(new Fluorophore(x, y, imN, 
                config.fluorophoreConfig));
        }
    } else
        std::cerr << "Fluorophore generation error: Unknown pattern "
             << config.fluorophoreDistribution() << "\n";
}

template <typename Pixel>
NoiseSource<Pixel>::~NoiseSource()
    throw()
{
    /* The last value of our random number generator is the next
     * seed. */
    randomSeedEntry = gsl_rng_get(rng);
    gsl_rng_free(rng);
    foreach( fl, std::list<Fluorophore*>, fluorophores ) 
        delete *fl;
}

template <typename Pixel>
std::auto_ptr< CImgBuffer::Image<Pixel> >
NoiseSource<Pixel>::fetch( int imNum )
    throw()
{
    std::auto_ptr< CImgBuffer::Image<Pixel> > result
        (new CImgBuffer::Image<Pixel>(imW, imH, imNum));

    noiseGenerator->pixelNoise(result->data, result->size());

    /* Then add the fluorophores. */
    PROGRESS("Making glare for " << fluorophores.size() << 
             " fluorophores");
    foreach( fl, std::list<Fluorophore*>, fluorophores ) 
        (*fl)->glareInImage(rng, *result, 0.1);

    return result;
}

template class NoiseConfig<dStorm::StormPixel>;
template class NoiseSource<dStorm::StormPixel>;

}

#define LOCPREC_NOISEGENERATOR_CPP
#include "NoiseGenerator.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>
#include <fstream>
#include <math.h>
#include <cassert>
#include <time.h>

using namespace std;

typedef unsigned short Pixel;

namespace input_simulation {

template <typename PixelType> class CombinedDistributionSource
: public NoiseGenerator<PixelType>
{
  private:
    double G, mu, sigma, x0, theta;
    gsl_rng *rng;
  protected:
    void pixelNoise(PixelType *vec, size_t size);

  public:
    CombinedDistributionSource(const NoiseGeneratorConfig &config,
        gsl_rng *rng);
    ~CombinedDistributionSource();
};

template <typename PixelType> class MeasuredNoiseSource
: public NoiseGenerator<PixelType>
{
  private:
    PixelType *pixels;
    unsigned int *weights, total;
    unsigned int valueCount, divisor;
    gsl_rng *rng;

  protected:
    void pixelNoise(PixelType *vec, size_t size);

  public:
    MeasuredNoiseSource(const NoiseGeneratorConfig &config,
        gsl_rng *rng);
    ~MeasuredNoiseSource();
};

/*template <typename PixelType> class MeasuredDistributionSource
: public NoiseGenerator<PixelType>
{
};*/

_NoiseGeneratorConfig::_NoiseGeneratorConfig() 
: simparm::Set("NoiseGenerator", "Generate with random noise"),
  ups_G("UpsilonG", "Value of G in combined PDF", 0.96),
  ups_mu("UpsilonMu", "Value of mu in combined PDF", 2734.6),
  ups_sigma("UpsilonSigma", "Value of sigma in combined PDF", 104.6),
  ups_x0("UpsilonX0", "Value of x0 in combined PDF", 2582.6),
  ups_theta("UpsilonTheta", "Value of theta in combined PDF", 119.7),
  noiseFile("NoiseFile", "File to load noise data from"),
  varianceScale("ScaleVariance", "Scale variance on measured "
                "noise by factor", 1),
  width("ImageWidth", "Width of generated source image", 256),
  height("ImageHeight", "Height of generated source image", 256),
  random_seed("RandomSeed", "Random seed", time(NULL))
{
}

void _NoiseGeneratorConfig::registerNamedEntries() {
    push_back(ups_G);
    push_back(ups_mu);
    push_back(ups_sigma);
    push_back(ups_x0);
    push_back(ups_theta);
    push_back(width);
    push_back(height);
    push_back(random_seed);
    push_back(noiseFile);
    push_back(varianceScale);
}

template<>
CombinedDistributionSource<Pixel>::CombinedDistributionSource
    (const NoiseGeneratorConfig &c, gsl_rng *rng)
: rng(rng)
{
    G = c.ups_G();
    mu = c.ups_mu();
    sigma = c.ups_sigma();
    x0 = c.ups_x0();
    theta = c.ups_theta();
}

template<>
MeasuredNoiseSource<Pixel>::MeasuredNoiseSource
    (const NoiseGeneratorConfig &config, gsl_rng *rng)
: rng(rng)
{
    assert( gsl_rng_min(rng) == 0 );
    unsigned int max = gsl_rng_max(rng);
    total = 0;
    ifstream f(config.noiseFile().c_str(), ios_base::in);
    list< pair<Pixel, unsigned int> > pixelList;
    while (f) {
        unsigned int val, count;
        f >> val >> count;
        if (f) {
            pixelList.push_back(
                pair<unsigned int,unsigned int>(val, count));
            /* Avoid overflow */
            if (total + count > total)
                total += count;
            else
                cerr << "Warning: Integer overflow in MeasuredNoiseSource constructor."
                     << endl;
        }
    }
    if ( total == 0 ) throw std::runtime_error("The noise file has no entries");
    
    valueCount = pixelList.size();
    double d_pixels[valueCount], d_weights[valueCount];

    weights = (unsigned int*)malloc(sizeof(unsigned int) * valueCount);
    this->pixels = (Pixel*)malloc(sizeof(Pixel) * valueCount);
    int api = 0;
    for (list< pair<Pixel, unsigned int> >::iterator i = pixelList.begin();
         i != pixelList.end(); i++)
    {
        d_weights[api] = weights[api] = i->second;
        d_pixels[api] = this->pixels[api] = i->first;
        api++;
    }

    double mean = gsl_stats_wmean(d_weights, 1, d_pixels, 1, api);
    double varScale = config.varianceScale();
    for (unsigned int i = 0; i < valueCount; i++) {
        pixels[i] = (Pixel)round((pixels[i] - mean) * varScale + mean);
    }

    divisor = 1;
    unsigned int tryDivisor = 2;
    while (max % tryDivisor == tryDivisor-1 && max / tryDivisor >= total) {
        divisor = tryDivisor;
        tryDivisor *= 2;
    }
}

template<>
std::auto_ptr<NoiseGenerator<Pixel> > 
NoiseGenerator<Pixel>::factory(const NoiseGeneratorConfig &config,
    gsl_rng *rng)
 
{
    NoiseGenerator<Pixel> *r = NULL;
    if (config.noiseFile)
        r = new MeasuredNoiseSource<Pixel>(config, rng);
    else
        r = new CombinedDistributionSource<Pixel>(config, rng);
    return std::auto_ptr<NoiseGenerator<Pixel> >(r);
}

template <>
CombinedDistributionSource<Pixel>::~CombinedDistributionSource(){}

template <>
void CombinedDistributionSource<Pixel>::pixelNoise
    (Pixel *vec, size_t size)
{
    /* Fill the target image with \Upsilon noise. */
    for (unsigned int i = 0; i < size; i++) {
        if (gsl_rng_uniform(rng) < G)
            vec[i] = gsl_ran_gaussian(rng, sigma) + mu;
        else
            vec[i] = gsl_ran_gamma(rng, 2, theta) + x0;
    }
}

template <>
MeasuredNoiseSource<Pixel>::~MeasuredNoiseSource() {
    free(weights);
    free(pixels);
}

template <>
void MeasuredNoiseSource<Pixel>::pixelNoise
    (Pixel *vec, size_t size)
{
    for (unsigned int i = 0; i < size; i++) {
      retry:
        unsigned int rn = gsl_rng_get(rng) / divisor;
        unsigned int it = 0;
        while ( it < valueCount ) {
            if (rn < weights[it]) {
                vec[i] = pixels[it];
                goto nextPixel;
            } else {
                rn -= weights[it];
                it++;
            }
        }
        goto retry;

      nextPixel: ;
    }
}


}

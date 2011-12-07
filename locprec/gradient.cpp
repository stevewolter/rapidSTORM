#include <gsl/gsl_randist.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_statistics_double.h>
#include <dStorm/input/Config.h>
#include <dStorm/image/iterator.h>
#include "NoiseSource.h"
#include <iostream>
#include <stdint.h>
#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <Eigen/Core>

using namespace locprec;

int main(int argc, char *argv[]) {
    dStorm::input::Config inputConfig;
    simparm::Entry<unsigned long>Entry runs("Runs", "Number of runs to average over", 1); 
    NoiseConfig<uint16_t> config( inputConfig );
    simparm::Set cmdline("cmdline", "");
    cmdline.push_back(config);
    cmdline.push_back(runs);
    cmdline.readConfig( argc, argv );

    int check_2_power = config.imageNumber();
    while ( check_2_power % 2 == 0 ) check_2_power /= 2;
    if ( check_2_power != 1 ) {
        std::cerr << "Image number must be a power of 2" << std::endl;
        return 1;
    }

    int N = config.imageNumber(), naive_count = 300;
    Eigen::VectorXd signals(N), store(N), ps(N),
                    autocorr(naive_count),
                    means = Eigen::VectorXd::Zero(naive_count),
                    M2s = Eigen::VectorXd::Zero(naive_count);
    Eigen::MatrixXd naked_autocorr = Eigen::MatrixXd::Zero(naive_count, runs());

  for (unsigned int run = 0; run < runs(); ++run) {
    NoiseSource<uint16_t> source( config );

    int k = 0;
    typedef NoiseSource<uint16_t>::base_iterator iterator;
    for ( iterator i = source.begin(), end = source.end(); i != end; ++i )
    {
        double sum = 0;
        for ( iterator::value_type::const_iterator j = i->begin();
              j != i->end(); ++j )
            sum += *j;
        signals[k++] = sum;
    }
    store = signals;

    double mean, stddev;
    mean = gsl_stats_mean( signals.data(), 1, N );
    //stddev = gsl_stats_sd_m( signals.data(), 1, N, mean );
    //signals = (signals.cwise() - mean) / stddev;

    for (int i = 0; i < naive_count; ++i) {
        for (int x = 0; x < N-i; ++x) 
            naked_autocorr(i,run) += (signals[x] - mean) * (signals[x+i] - mean) / (mean*mean) / (N-i);
    }

#if 0
    gsl_fft_real_radix2_transform( signals.data(), 1, N );
    //std::cout << 0 << " " << signals[0] << " 0\n";
    for ( int i = 1; i < N-2; i += 2 ) {
        std::cout << N*1.0/((i+1)/2) << " " << signals[i] << " " << signals[i+1] << "\n";
    }
    std::cout << N/(N/2.0) << " " << signals[N-1] << " 0\n";

    ps[0] = signals[0] = signals[0] * signals[0];
    for ( int i = 1; i < N-2; i += 2 ) {
        /* i is the real part, i+1 the complex part */
        ps[(i+1)/2] = ps[N-1-(i+1)/2] = signals[i] = signals[i] * signals[i] + signals[i+1] * signals[i+1];
        signals[i+1] = 0;
    }
    ps[N/2] = signals[N-1] = signals[N-1] * signals[N-1];
    gsl_fft_halfcomplex_radix2_inverse( signals.data(), 1, N );
#endif
  }
    std::cout << naked_autocorr;

    return 0;
}

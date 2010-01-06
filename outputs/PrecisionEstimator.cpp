#define LOCPREC_PRECISIONESTIMATOR_CPP
#include "PrecisionEstimator.h"
#include <iomanip>
#include <fit++/Exponential2D.hh>
#include <fit++/FitFunction_impl.hh>
#include <fit++/Exponential2D_impl.hh>
#include <fit++/Exponential2D_Correlated_Derivatives.hh>
#include <dStorm/output/Trace.h>
#include <sstream>
#include <dStorm/helpers/Variance.h>
#include <gsl/gsl_statistics_double.h>
#include <dStorm/input/ImageTraits.h>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/units/systems/si/length.hpp>

using namespace std;
using namespace fitpp;
using namespace boost::units;

typedef quantity<
    make_scaled_unit<
        si::length,
        scale<10, static_rational<-9> > >::type>
    nanometre_length;

namespace dStorm {
namespace output {

namespace Precision {
struct FitSigmas { 
    quantity<camera::length> x, y;
    double xy; int n; double a; 
};
}

SinglePrecisionEstimator::_Config::_Config()
: simparm::Object("SeperatePrecision", 
                  "Estimate localization precision per spot"),
  outputFile("ToFile", "Save precision info to") 
{}

MultiPrecisionEstimator::_Config::_Config()
: simparm::Object("OverallPrecision", 
                  "Estimate overall localization precision")
{}

SinglePrecisionEstimator::SinglePrecisionEstimator
    ( const SinglePrecisionEstimator::Config& config )
: OutputObject("SinglePrecisionEstimator", "Per-spot precision estimator"),
  printTo( config.outputFile ),
  res_enh(1/config.resEnh())
{}

MultiPrecisionEstimator::MultiPrecisionEstimator
    ( const MultiPrecisionEstimator::Config& config )
: OutputObject("MultiPrecisionEstimator", 
            "Overall precision estimation"),
    usedSpots("UsedSpots", "# of localizations used",
            0),
    x_sd("XSD", "X FWHM in nm", 0),
    y_sd("YSD", "Y FWHM in nm", 0),
    corr("Corr", "Correlation between axes", 0),
    res_enh(1/config.resEnh())
{ registerNamedEntries(); }

MultiPrecisionEstimator::MultiPrecisionEstimator
    ( const MultiPrecisionEstimator& c )
: OutputObject(c), 
  usedSpots(c.usedSpots),
  x_sd(c.x_sd), y_sd(c.y_sd), corr(c.corr),
  pixel_dim(c.pixel_dim), res_enh(c.res_enh)
{ registerNamedEntries(); }

SinglePrecisionEstimator* SinglePrecisionEstimator::clone() const
    { return new SinglePrecisionEstimator(*this); }

MultiPrecisionEstimator* MultiPrecisionEstimator::clone() const
    { return new MultiPrecisionEstimator(*this); }

using namespace Precision;

quantity<camera::length> 
compute_weighted_SD( const Trace& trace, int coordinate )
{
    typedef quantity< boost::units::unit<boost::units::length_dimension, camera::system> > Coord;
    /* West algorithm as pseudocoded on 
     * http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance */
    int n = 0;
    Coord mean = 0;
    quantity<camera::area> S = 0;
    double sumweight = 0;
    for ( Trace::const_iterator i = trace.begin();
          i != trace.end(); i++)
    {
        Coord x = i->position()[coordinate];
        double weight = i->getStrength();
        if (n == 0) {
            n = 1;
            mean = x;
            S = 0;
            sumweight = weight;
        } else {
            n++;
            double temp = weight + sumweight;
            Coord Q = x - mean;
            Coord R = Q * weight / temp;
            S += sumweight * (Q * R);
            mean += R;
            sumweight = temp;
        }
    }
    return sqrt( S * double(n / ((n-1) * sumweight)) );
}

Output::AdditionalData
SinglePrecisionEstimator::announceStormSize(const Announcement& a)
{
    /* Length of a pixel in nm is inverse of number of dots per nm. */
    pixel_dim = a.traits.resolution;
    return AdditionalData().set_cluster_sources();
}

Output::Result
SinglePrecisionEstimator::receiveLocalizations( const EngineResult &er )
 
{
    ost::MutexLock lock(mutex);
    try {
        for ( int i = 0; i < er.number; i++ ) {
            const Localization& l = er.first[i];
            assert( l.has_source_trace() );

            FitSigmas s = fitWithGauss( res_enh, &l, 1);
            printTo.get_output_stream()
                << setw(10) << fixed << setprecision(2) << l.x().value()
                << setw(10) << l.y().value()
                << setw(5)  << l.get_source_trace().size()
                << setw(10) << setprecision(2) << 
                    nanometre_length(
                        compute_weighted_SD(
                            l.get_source_trace(), 0) 
                            *2.35* pixel_dim.x()
                    ).value()
                << setw(10) << setprecision(2) <<
                    nanometre_length(
                        compute_weighted_SD(
                            l.get_source_trace(), 1)
                        *2.35* pixel_dim.y() ).value()
                << setw(10) << setprecision(2) << 
                    nanometre_length( 
                        s.x *2.35* pixel_dim.x()
                    ).value()
                << setw(10) << setprecision(2) <<
                    nanometre_length( 
                        s.y *2.35* pixel_dim.y()
                    ).value()
                << setw(10) << setprecision(3) << s.xy
                << "\n";
        }
    } catch (const std::exception& e) {
        cerr << "Error in precision estimation: " 
             << e.what() << endl;
    }

    return KeepRunning;
}

void SinglePrecisionEstimator::propagate_signal
    (Output::ProgressSignal s) 
{
    if ( s == Engine_is_restarted || s == Engine_run_succeeded ) {
        ost::MutexLock lock(mutex);
        printTo.close_output_stream();
    }
}

void MultiPrecisionEstimator::registerNamedEntries() {
    usedSpots.editable = false;
    push_back( usedSpots );
    x_sd.editable = false;
    push_back( x_sd );
    y_sd.editable = false;
    push_back( y_sd );
    corr.editable = false;
    push_back( corr );
}

Output::AdditionalData
MultiPrecisionEstimator::announceStormSize(const Announcement& a)
{
    pixel_dim = a.traits.resolution;
    return localizations.announceStormSize(a);
}


void MultiPrecisionEstimator::estimatePrecision() {
    ost::MutexLock lock(mutex);
    FitSigmas s;
    output::Localizations& locs = localizations.getResults();

    if (locs.binNumber() == 1)
        s = fitWithGauss( res_enh, locs.getBin(0), locs.sizeOfBin(0) );
    else {
        int number = locs.size();
        data_cpp::Vector<Localization> v( number );
        for (int bin = 0; bin < locs.binNumber(); bin++) {
            for (int i = 0; i < locs.sizeOfBin(bin); i++) {
                v.push_back( locs.getBin(bin)[i] );
            }
        }

        s = fitWithGauss( res_enh, v.ptr(), number );
    }

    usedSpots = s.n;
    x_sd = nanometre_length(s.x * 2.35 * pixel_dim.x())
            .value();
    y_sd = nanometre_length(s.y * 2.35 * pixel_dim.y())
            .value();
    corr = s.xy;
    if ( ! x_sd.isActive() ) std::cout << x_sd() << " " << y_sd() << "\n";
}

FitSigmas Precision::fitWithGauss
   (double res_enh, const Localization* first, int number) 
{
    typedef data_cpp::Vector<Localization> Points;
    FitSigmas result;

    /* data_range gives the maximal L_infty distance of a point
        * to the localization centre. */
    quantity<camera::length, float> data_range = 0;
    int total_count = 0;
    dStorm::Variance< quantity<camera::length>,
                      quantity<camera::area> >
        average_sd_x, average_sd_y;
    for (int j = 0; j < number; j++) {
        const Localization *i = first + j;
        if (i->get_source_trace().size() == 0) continue;

        average_sd_x.addValue( 
            compute_weighted_SD( i->get_source_trace(), 0 ) );
        average_sd_y.addValue( 
            compute_weighted_SD( i->get_source_trace(), 1 ) );

        const Points& p = i->get_source_trace();
        for ( Points::const_iterator k = p.begin(); k != p.end(); k++)
        {
            data_range = max( data_range,
                max( abs( k->x() - i->x() ), abs( k->y() - i->y() ) ) );
        }

        total_count += i->get_source_trace().size();
    }
    if (total_count == 0) return result;

    int center_bin = 
        int(ceil(res_enh * data_range.value()));
    int bin_number = 2*center_bin+1;

    double data[bin_number][bin_number];
    for (int x = 0; x < bin_number; x++)
        for (int y = 0; y < bin_number; y++)
            data[y][x] = 0;

    double norm = 1.0 / total_count;
    const int ExpFlags = Exponential2D::FreeForm & (~Exponential2D::Shift);

    Exponential2D::For<1,ExpFlags>::FitObject<double> fitter;
    fitter.setSize(bin_number, bin_number);
    fitter.setData((const double*)data, 1, bin_number);

    result.n = total_count;

    fitter.setSigmaX<0>(
        res_enh * average_sd_x.mean().value() );
    fitter.setSigmaY<0>( 
        res_enh * average_sd_y.mean().value() );
    fitter.setSigmaXY<0>( 0 );
    fitter.setShift( 0 );
    fitter.setAmplitude<0>( 1 );

    double max_x_val = -1, max_y_val = -1;
    int max_x_bin = 0, max_y_bin = 0;
    /* Fill the data vector with the number of points in that bin. */
    for ( const Localization* r = first; r < first + number; r++ ) {
        const Points& ps = r->get_source_trace();
        for ( Points::const_iterator p = ps.begin(); p != ps.end(); p++)
        {
            double x_off = (p->x() - r->x()) / camera::pixel * res_enh,
                   y_off = (p->y() - r->y()).value() * res_enh;
            int x_bin = int(round(x_off)) + center_bin,
                y_bin = int(round(y_off)) + center_bin;
            if ( y_bin >= 0 && y_bin < bin_number && 
                    x_bin >= 0 && x_bin < bin_number )
                data[y_bin][x_bin] += norm;
            
            if ( norm > max_x_val ) max_x_bin = x_bin;
            if ( norm > max_y_val ) max_y_bin = y_bin;
        }
    }

    fitter.setMeanX<0>( center_bin );
    fitter.setMeanY<0>( center_bin );

    fitter.fit();

    result.x = 
        fitter.getSigmaX<0>() * camera::pixel / res_enh;
    result.y =
        fitter.getSigmaY<0>() * camera::pixel / res_enh;
    result.xy = fitter.getSigmaXY<0>() / res_enh;
    result.a = fitter.getAmplitude<0>();

    return result;
}


}
}

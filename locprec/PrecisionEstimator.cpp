#define LOCPREC_PRECISIONESTIMATOR_CPP
#include "PrecisionEstimator.h"
#include "foreach.h"
#include <iomanip>
#include <fit++/Exponential2D.hh>
#include <dStorm/Trace.h>

using namespace std;
using namespace dStorm;
using namespace libfitpp;

namespace locprec {

Transmission::Result
PrecisionEstimator::receiveLocalizations( const EngineResult &er )
throw() 
{
    try {
        unsigned int total = 0;
        for ( int i = 0; i < er.number; i++ ) {
            const Localization& l = er.first[i];
            assert( l.has_source_trace() );

            FitSigmas s = fitWithGauss( &l, 1);
            if (printSingle)
                (*printSingle)
                    << setw(10) << l.x()
                    << setw(10) << l.y()
                    << setw(5)  << l.get_source_trace().size()
                    << setw(10) << l.get_source_trace().get_X_SD()
                    << setw(10) << l.get_source_trace().get_Y_SD()
                    << setw(11) << s.x 
                    << setw(11) << s.y 
                    << setw(11) << s.xy 
                    << setw(11) << s.x * 2.35 * nm
                    << setw(11) <<  s.y * 2.35 * nm
                    << "\n";
            if (printSpots) {
                (*printSpots) << "\n\n";
                foreach_const( l, data_cpp::Vector<Localization>,
                            er.first[i].get_source_trace() )
                    (*printSpots) << *l;
            }
        }
        if (printMulti) {
            FitSigmas s = fitWithGauss( er.first, er.number );
            (*printMulti)
                << setw(25) << total 
                << setw(11) << s.x 
                << " (" << s.x * 2.35 * nm << " nm)"
                << setw(11)<< s.y 
                << " (" << s.y * 2.35 * nm << " nm)"
                << setw(11)<< s.xy 
                << " (" << s.xy * 2.35 * nm << " nm)"
                << "\n";
        }
    } catch (const std::exception& e) {
        cerr << "Error in precision estimation: " 
             << e.what() << endl;
    }

    return KeepRunning;
}

PrecisionEstimator::FitSigmas PrecisionEstimator::fitWithGauss
   (const Localization* first, int number) throw() 
{
    FitSigmas result;

    /* data_range gives the maximal L_infty distance of a point
        * to the localization centre. */
    double data_range = 0;
    int total_count = 0;
    for (int j = 0; j < number; j++) {
        const Localization *i = first + j;
        if (i->get_source_trace().size() == 0) continue;

        foreach_const( k, data_cpp::Vector<Localization>, 
                          i->get_source_trace() )
        {
            data_range = max( data_range,
                max( abs( k->x() - i->x() ), abs( k->y() - i->y() ) ) );
        }

        total_count += i->get_source_trace().size();
    }
    if (total_count == 0) return result;

    int center_bin = int(ceil(res_enh * data_range));
    int bin_number = 2*center_bin+1;

    double data[bin_number][bin_number];
    for (int x = 0; x < bin_number; x++)
        for (int y = 0; y < bin_number; y++)
            data[y][x] = 0;

    double norm = 1.0 / total_count;
    Exponential2D<double,6> fitter;
    fitter.setSize(bin_number, bin_number);
    for (int i = 0; i < 7; i++)
        fitter.setVariableness(i, (i != Exponential2D<double,6>::Shift));
    fitter.setData((const double*)data,
        1, bin_number, 0, bin_number, 0, bin_number);

    result.n = total_count;

    fitter.setSigmaX( 2 );
    fitter.setSigmaY( 2 );
    fitter.setSigmaXY( 0 );
    fitter.setShift( 0 );
    fitter.setAmplitude( 1 );

    double max_x_val = -1, max_y_val = -1;
    int max_x_bin = 0, max_y_bin = 0;
    /* Fill the data vector with the number of points in that bin. */
    for ( const Localization* r = first; r < first + number; r++ ) {
        foreach_const( p, data_cpp::Vector<Localization>,
                       r->get_source_trace() ) 
        {
            double x_off = (p->x() - r->x()) * res_enh,
                   y_off = (p->y() - r->y()) * res_enh;
            int x_bin = int(round(x_off)) + center_bin,
                y_bin = int(round(y_off)) + center_bin;
            if ( y_bin >= 0 && y_bin < bin_number && 
                    x_bin >= 0 && x_bin < bin_number )
                data[y_bin][x_bin] += norm;
            
            if ( norm > max_x_val ) max_x_bin = x_bin;
            if ( norm > max_y_val ) max_y_bin = y_bin;
        }
    }

    fitter.setMeanX( max_x_bin );
    fitter.setMeanY( max_y_bin );

    fitter.fit();

    result.x = fitter.getSigmaX() / res_enh;
    result.y = fitter.getSigmaY() / res_enh;
    result.xy = fitter.getSigmaXY() / res_enh;
    result.a = fitter.getAmplitude();

    return result;
}


}


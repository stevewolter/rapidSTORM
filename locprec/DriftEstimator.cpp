#define LOCPREC_DRIFTESTIMATOR_CPP
#include "DriftEstimator.h"
#include "foreach.h"
#include <gsl/gsl_statistics.h>
#include <limits>

using namespace std;
using namespace dStorm;

namespace locprec {

DriftEstimator::DriftEstimator (const RegionSegmenterConfig &c) 
: RegionSegmenter(c)
{
}

struct PointDistance {
    union {
        const MoleculeLocalization *p;
        double ph;
    } src;
    double mean[2], deviation[2];
    double samples;
};

/** \return The number of points the two fit lists had in common. */
static PointDistance pdistance(const MoleculeLocalization& as,
                              const MoleculeLocalization& bs)
{
    const list<Localization>& a = as.getPoints(), &b = bs.getPoints();
    int maxCount = max<int>(a.size(), b.size());
    double x_distances[maxCount], y_distances[maxCount];
    int samples = 0;
    list<Localization>::const_iterator ai = a.begin(), bi = b.begin();
    while ( ai != a.end() && bi != b.end() ) {
        int diff = ai->N() - bi->N();
        if ( diff == 0 ) {
            x_distances[samples] = ai->x() - bi->x();
            y_distances[samples] = ai->y() - bi->y();
            ai++, bi++;
            samples++;
        } else if (diff < 0)
            ai++;
          else
            bi++;
    }
    PointDistance rv;
    rv.src.p = &as;
    rv.mean[0] = gsl_stats_mean(x_distances, 1, samples);
    rv.mean[1] = gsl_stats_mean(y_distances, 1, samples);
    rv.deviation[0] = gsl_stats_sd_m(x_distances, 1, samples, rv.mean[0]);
    rv.deviation[1] = gsl_stats_sd_m(y_distances, 1, samples, rv.mean[1]);
    rv.samples = samples;
    return rv;
}

static const int stride = sizeof(PointDistance)/sizeof(double);

static void computeBestPivot(const list<MoleculeLocalization>& regions,
                      PointDistance target[], int& minDistN)
{
    int count = regions.size();
    double bestAverageDeviation = numeric_limits<double>::infinity();
    foreach_const( maxI, list<MoleculeLocalization>, regions ) {
        PointDistance distances[count];
        int distanceN = 0;
        foreach_const( i, list<MoleculeLocalization>, regions ) {
            PointDistance d = pdistance( *i, *maxI );
            distances[distanceN] = d;
            distanceN++;
        }
        double avDeviation = 
            ( gsl_stats_mean(&distances[0].deviation[0], stride, distanceN)
             +gsl_stats_mean(&distances[0].deviation[1], stride, distanceN)
            ) / 2;
        if (avDeviation < bestAverageDeviation) {
            memcpy(target, distances, sizeof(PointDistance) * count);
            bestAverageDeviation = avDeviation;
            minDistN = distanceN;
        }
    }
}

static void thresholdByAverage( PointDistance ds[], int& dc )
{
    double xav = gsl_stats_mean(&ds->deviation[0], stride, dc),
           yav = gsl_stats_mean(&ds->deviation[1], stride, dc);

    int shift = 0;
    for (int i = 0; i < dc; i++) {
        if ( ds[i].deviation[0] > xav || ds[i].deviation[1] > yav )
            shift++;
        else if (shift > 0)
            ds[i-shift] = ds[i];
    }
    dc -= shift;
}

#if 0
static void printDeviations( PointDistance ds[], int dc ) {
    for (int i = 0; i < dc; i++) {
        PointDistance &pd = ds[i];
        cerr << pd.src.p->centerX() << " " << pd.src.p->centerY() << " " 
             << pd.mean[0] << " " << pd.mean[1] << " " 
             << pd.deviation[0] << " " << pd.deviation[1] << endl;
    }
}
#endif

static auto_ptr<list<Localization> > deconvolute( PointDistance ds[], int dc ) 

{
    auto_ptr< list<Localization> > rv( new list<Localization>() );
    for (int i = 0; i < dc; i++) {
        foreach_const( j, list<Localization>, 
                       ds[i].src.p->getPoints() )
        {
            Localization f = *j;
            f.shiftX( - ds[i].mean[0] );
            f.shiftY( - ds[i].mean[1] );
            rv->push_back(f);
        }
    }
    return rv;
}

static bool smallerN(const Localization &a, const Localization &b) {
    return a.N() < b.N();
}

#if 0
static void printFitList(list<Localization>& l) {
    foreach_const( i, list<Localization>, l ) {
        cout << i->x() << " " << i->y() << " " << i->N() << endl;
    }
}
#endif

static vector< pair<double,double> > average( const list<Localization>& sorted )

{
    if (sorted.size() == 0) return vector<pair<double,double> >();
    int max = sorted.back().N();
    vector< pair<double,double> > rv(max+1);
    list<Localization>::const_iterator i = sorted.begin();
    for (int cur = 0; cur <= max; cur++) {
        rv[cur].first = 0;
        rv[cur].second = 0;
        int samples = 0;
        while ( i->N() == cur ) {
            rv[cur].first += i->x();
            rv[cur].second += i->y();
            samples++;
            i++;
        }
        rv[cur].first /= samples;
        rv[cur].second /= samples;
    }
    return rv;
}

void DriftEstimator::carStopped() {
    segment();

    PointDistance minDistances[getRegions().size()];
    int minDistanceN;
    computeBestPivot( getRegions(), minDistances, minDistanceN );
    thresholdByAverage( minDistances, minDistanceN );
    auto_ptr< list<Localization> > deconvoluted 
        = deconvolute( minDistances, minDistanceN );
    deconvoluted->sort( smallerN );
    vector< pair<double,double> > correction = average( *deconvoluted );

    double minX , minY, maxX, maxY;
    minX = maxX = correction[0].first;
    minY = maxY = correction[0].second;
    for (unsigned int i = 1; i < correction.size(); i++) {
        if (minX > correction[i].first) minX = correction[i].first;
        if (maxX < correction[i].first) maxX = correction[i].first;
        if (minY > correction[i].second) minY = correction[i].second;
        if (maxY < correction[i].second) maxY = correction[i].second;
    }
    cout << ceil(maxX - minX) << " " << ceil(maxY - minY) << " "
         << correction.size() << endl;
    for (unsigned int i = 0; i < correction.size(); i++)
        cout << correction[i].first - minX << " " 
             << correction[i].second - minY << " "
             << i << " "
             << endl;
}

}

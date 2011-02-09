#define LOCPREC_PRECISIONESTIMATOR_CPP
#include <fit++/Exponential2D.hh>
#include <fit++/FitFunction_impl.hh>
#include <fit++/Exponential2D_impl.hh>
#include <fit++/Exponential2D_Correlated_Derivatives.hh>
#include "PrecisionEstimator.h"
#include <dStorm/output/Trace.h>
#include <iomanip>
#include <sstream>
#include <dStorm/helpers/Variance.h>
#include <dStorm/ImageTraits.h>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/units/unit.hpp>
#include <dStorm/units/nanolength.h>
#include <boost/units/systems/camera/luminance.hpp>
#include <boost/units/systems/si/area.hpp>
#include <dStorm/output/Localizations_iterator.h>

using namespace std;
using namespace fitpp;
using namespace boost::units;

namespace dStorm {
namespace output {

namespace Precision {
struct FitSigmas { 
    quantity<boost::units::si::length> x, y;
    double xy; int n; double a; 
};

class GaussFitter {
    /* data_range gives the maximal L_infty distance of a point
        * to the localization centre. */
    double res_enh;
    quantity<si::length, float> data_range;
    int total_count, center_bin;
    dStorm::Variance< quantity<si::length>, quantity<si::area> >
        average_sd_x, average_sd_y;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor | Eigen::DontAlign > DataMatrix;
    DataMatrix data;

    void first_pass( const Localization& l ); 
    void second_pass( const Localization& l ); 
    void init_fit_data();
    FitSigmas perform_fit();

  public:
    GaussFitter(double res_enh) : res_enh(res_enh) {}
    template <typename Iterator>
    FitSigmas process( Iterator begin, Iterator end ) {
        data_range = 0 * si::meter; total_count = 0;
        for ( Iterator i = begin; i != end; ++i )
            first_pass(*i);
        if ( total_count == 0 ) return FitSigmas();
        init_fit_data();
        for ( ; begin != end; ++begin )
            second_pass(*begin);
        return perform_fit();
    }
};

}

SinglePrecisionEstimator::_Config::_Config()
: simparm::Object("SeperatePrecision", 
                  "Estimate localization precision per spot"),
  outputFile("ToFile", "Save precision info to", ".txt") 
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
  res_enh(c.res_enh)
{ registerNamedEntries(); }

SinglePrecisionEstimator* SinglePrecisionEstimator::clone() const
    { return new SinglePrecisionEstimator(*this); }

MultiPrecisionEstimator* MultiPrecisionEstimator::clone() const
    { return new MultiPrecisionEstimator(*this); }

using namespace Precision;

quantity<boost::units::si::length> 
compute_weighted_SD( const Trace& trace, int coordinate )
{
    typedef quantity< boost::units::si::length > Coord;
    typedef quantity< camera::intensity > Weight;

    typedef boost::units::multiply_typeof_helper< 
                boost::units::multiply_typeof_helper<
                    Coord::unit_type,Coord::unit_type >::type,
                Weight::unit_type>::type 
        weight_area;

    /* West algorithm as pseudocoded on 
     * http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance */
    int n = 0;
    Coord mean = 0;
    quantity<weight_area> S = 0;
    Weight sumweight = 0;
    for ( Trace::const_iterator i = trace.begin();
          i != trace.end(); i++)
    {
        Coord x = i->position()[coordinate];
        Weight weight = i->amplitude();
        if (n == 0) {
            n = 1;
            mean = x;
            S = 0;
            sumweight = weight;
        } else {
            n++;
            Weight temp = weight + sumweight;
            Coord Q = x - mean;
            Coord R = Q * weight / temp;
            S += sumweight * (Q * R);
            mean += R;
            sumweight = temp;
        }
    }
    return sqrt( S * (Weight::value_type(double(n) / n-1) / sumweight) );
}

Output::AdditionalData
SinglePrecisionEstimator::announceStormSize(const Announcement& a)
{
    /* Length of a pixel in nm is inverse of number of dots per nm. */
    return AdditionalData().set_cluster_sources();
}

Output::Result
SinglePrecisionEstimator::receiveLocalizations( const EngineResult &er )
 
{
    ost::MutexLock lock(mutex);
    try {
        Precision::GaussFitter fitter( res_enh );
        for ( EngineResult::const_iterator i = er.begin(); i != er.end(); ++i ) {
            EngineResult::const_iterator next = i; ++next;
            FitSigmas s = fitter.process(i, next);
            printTo.get_output_stream()
                << setw(10) << fixed << setprecision(2) << i->position().x().value()
                << setw(10) << i->position().y().value()
                << setw(5)  << i->get_source_trace().size()
                << setw(10) << setprecision(2) << 
                    (compute_weighted_SD(
                        i->get_source_trace(), 0) *2.35  ) / si::nanometre
                << setw(10) << setprecision(2) <<
                    (compute_weighted_SD(
                            i->get_source_trace(), 1) *2.35 ) / si::nanometre
                << setw(10) << setprecision(2) << 
                    ( s.x *2.35 )/ si::nanometre
                << setw(10) << setprecision(2) <<
                    ( s.y *2.35 ) / si::nanometre
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
    return localizations.announceStormSize(a);
}


void MultiPrecisionEstimator::estimatePrecision() {
    ost::MutexLock lock(mutex);
    FitSigmas s;
    output::Localizations& locs = localizations.getResults();

    s = Precision::GaussFitter(res_enh).process( locs.begin(), locs.end() );

    usedSpots = s.n;
    x_sd = quantity<si::nanolength>(s.x * 2.35) / si::nanometer;
    y_sd = quantity<si::nanolength>(s.y * 2.35) / si::nanometer;
    corr = s.xy;
    if ( ! x_sd.isActive() ) std::cout << x_sd() << " " << y_sd() << "\n";
}

void GaussFitter::init_fit_data() {
    int center_bin = 
        int(ceil(res_enh * data_range.value()));
    int bin_number = 2*center_bin+1;

    data = DataMatrix::Zero( bin_number, bin_number );
}

FitSigmas GaussFitter::perform_fit() {

    const int ExpFlags = Exponential2D::FreeForm & (~Exponential2D::Shift);

    typedef Exponential2D::Model<1,ExpFlags> Model;
    Model::Constants constants;
    Model::Fitter<double,Eigen::Dynamic,Eigen::Dynamic,1>::Type fitter(constants);
    fitter.setSize(data.rows(), data.cols());
    fitter.setData(data.data(), data.rows(), data.cols(), 0);

    FitSigmas result;
    result.n = total_count;

    Exponential2D::Model<1,ExpFlags> model
        ( &fitter.getVariables(), &constants );
    model.setSigmaX<0>(
        res_enh * average_sd_x.mean().value() );
    model.setSigmaY<0>( 
        res_enh * average_sd_y.mean().value() );
    model.setSigmaXY<0>( 0 );
    model.setShift( 0 );
    model.setAmplitude<0>( 1 );

    model.setMeanX<0>( center_bin );
    model.setMeanY<0>( center_bin );

    fitpp::FitFunction<Model::VarC> fit_function;
    fitter.fit( fit_function );
    model.change_variable_set( &fitter.getVariables() );

    result.x = 
        model.getSigmaX<0>() * si::metre / res_enh;
    result.y =
        model.getSigmaY<0>() * si::metre / res_enh;
    result.xy = model.getSigmaXY<0>() / res_enh;
    result.a = model.getAmplitude<0>();

    return result;
}

void GaussFitter::first_pass(const Localization& l ) 
{
    if (l.get_source_trace().size() == 0) return;

    average_sd_x.addValue( 
        compute_weighted_SD( l.get_source_trace(), 0 ) );
    average_sd_y.addValue( 
        compute_weighted_SD( l.get_source_trace(), 1 ) );

    const output::Trace& p = l.get_source_trace();
    for ( output::Trace::const_iterator k = p.begin(); k != p.end(); k++)
    {
        data_range = max( data_range,
            max( abs( k->position().x() - l.position().x() ), abs( k->position().y() - l.position().y() ) ) );
    }

    total_count += l.get_source_trace().size();
}

void GaussFitter::second_pass(const Localization& r ) 
{
    const Trace& ps = r.get_source_trace();
    for ( Trace::const_iterator p = ps.begin(); p != ps.end(); p++)
    {
        double x_off = (p->position().x() - r.position().x()) / si::metre * res_enh,
                y_off = (p->position().y() - r.position().y()) / si::metre * res_enh;
        int x_bin = int(round(x_off)) + center_bin,
            y_bin = int(round(y_off)) + center_bin;
        if ( y_bin >= 0 && y_bin < data.rows() && 
                x_bin >= 0 && x_bin < data.cols() )
            this->data(y_bin, x_bin) += 1.0 / total_count;
    }
}


}
}

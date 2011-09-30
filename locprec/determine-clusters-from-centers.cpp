#include <Eigen/StdVector>
#include <dStorm/localization_file/reader.h>
#include <dStorm/unit_matrix_operators.h>
#include <dStorm/UnitEntries/Nanometre.h>
#include <dStorm/output/Localizations.h>
#include "PrecisionEstimator.h"
#include <boost/variant/get.hpp>
#include <Eigen/Array>

using namespace locprec;
using namespace boost::units;
using namespace boost::units::camera;
using dStorm::localization_file::Reader::Source;
using dStorm::localization_file::Reader::ChainLink;
using dStorm::localization::Record;

struct Config : public simparm::Set {
    PrecisionEstimator::Config config;
    simparm::FileEntry localizations, centers;
    dStorm::FloatNanometreEntry estimated_sigma;

    Config();
};

//Copy_if was dropped from the standard library by accident.
template<typename In, typename Out, typename Pred>
Out copy_if(In first, In last, Out res, Pred Pr)
{
  while (first != last)
  {
    if (Pr(*first))
      *res++ = *first;
    ++first;
  }
  return res;
}

class NearEnough {
    const quantity<si::nanolength, float> distance;
    const dStorm::input::Traits<dStorm::Localization>& resolution;
    const Eigen::Vector3f center;

  public:
    NearEnough( const ::Config& params, const dStorm::input::Traits<dStorm::Localization>& t, const dStorm::Localization& l, float scale ) 
        : distance(params.estimated_sigma() * scale),
          resolution(t),
          center(unitless_value(l.position())) {}
    bool operator()( const dStorm::Localization& l ) { 
        return ( ( unitless_value(l.position()) - center).squaredNorm() <= distance.value() * distance.value() );
    }
};

template <typename Type, int Columns>
Eigen::Matrix<Type, Eigen::Dynamic, Columns>
std_vector_to_matrix( 
    const std::vector< Eigen::Matrix<Type, Columns, 1> >& v
) {
    Eigen::Matrix<Type, Eigen::Dynamic, Columns> rv( v.size(), Columns );
    for (typename std::vector< Eigen::Matrix<Type, Columns, 1> >::const_iterator
          i = v.begin(); i != v.end(); ++i)
        rv.row( i - v.begin() ) = i->transpose();
    return rv;
}

Eigen::Matrix<float, Eigen::Dynamic, 3>
std_vector_to_matrix( 
    const std::vector< dStorm::Localization >& v, const dStorm::input::Traits<dStorm::Localization>& t
) {
    Eigen::Matrix<float, Eigen::Dynamic, 3> rv( v.size(), 3 );
    for (std::vector< dStorm::Localization >::const_iterator
          i = v.begin(); i != v.end(); ++i)
        rv.row( i - v.begin() ) = unitless_value(i->position()).transpose();
    return rv;
}

int process(int argc, char *argv[]) {
    ::Config params;
    params.readConfig( argc, argv );

    locprec::PrecisionEstimator e( params.config );
    dStorm::input::Traits<Record> context;

    if ( ! params.localizations ) throw std::runtime_error("No input file given.");
    if ( ! params.centers ) throw std::runtime_error("No centers file given.");
    std::auto_ptr<Source> source = ChainLink::read_file(params.localizations, context);

    Source::TraitsPtr traits_ptr = source->get_traits();
    assert( traits_ptr.get() != NULL );
    const dStorm::input::Traits<dStorm::Localization>& traits( *traits_ptr );
    std::vector<dStorm::Localization> localizations;
    for ( dStorm::input::Source<Record>::iterator i = source->begin(), e = source->end(); i != e; i++ ) {
        dStorm::Localization* l = boost::get<dStorm::Localization>(&*i);
        if ( l ) localizations.push_back(*l);
    }

    typedef std::vector< dStorm::Localization > Positions;
    Positions positions;
    std::istream& center_file( params.centers.get_input_stream() );
    while ( true ) {
        int n; float x, y;
        dStorm::Localization l;
        center_file >> n >> x >> y;
        l.position().x() = x * camera::pixel / *context.position().resolution().x();
        l.position().y() = y * camera::pixel / *context.position().resolution().y();
        if ( center_file )
            positions.push_back( l );
        else
            break;
    }

    for ( Positions::const_iterator i = positions.begin(); i != positions.end(); ++i ) 
    {
        NearEnough four_sigma(params, traits, *i, 2), one_sigma(params, traits, *i, 1);
        std::vector<dStorm::Localization> four_sigma_locs, one_sigma_locs;
        copy_if( localizations.begin(), localizations.end(), 
                 back_inserter(four_sigma_locs), four_sigma );
        copy_if( four_sigma_locs.begin(), four_sigma_locs.end(), 
                 back_inserter(one_sigma_locs), one_sigma );
        
        PrecisionEstimator::EstimationResult r 
            = e.estimate_deviation_from_initial_estimate( 
                std_vector_to_matrix( four_sigma_locs, traits ),
                locprec::PrecisionEstimator::SubSet( std_vector_to_matrix( one_sigma_locs, traits ) ) );
        std::cout << r.center.transpose() << " " << r.covariance.diagonal().cwise().sqrt().transpose() << std::endl;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    try {
        return process(argc, argv);
    } catch (std::exception& e) {
        std::cerr << e.what() << "\nUsage:\n"; ::Config().printHelp(std::cerr);
        return EXIT_FAILURE; 
    }
}

::Config::Config() 
: simparm::Set("DetermineClusters", "Determine clusters"),
  localizations("LocalizationFile", "Input file with localization data"),
  centers("CentersFile", "Input file with center pixel positions"),
  estimated_sigma("EstimatedSigma", "Estimated cluster standard deviation", 50 * boost::units::si::nanometre)
{
    push_back(config.h_value);
    push_back(config.confidence_interval);
    push_back(localizations);
    push_back(centers);
    push_back(estimated_sigma);
}

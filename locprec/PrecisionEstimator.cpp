#define LOCPREC_PRECISIONESTIMATOR_CPP

#include "PrecisionEstimator.h"

#include <iomanip>
#include <sstream>

#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>

#include <memory>

#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>

#include <Eigen/Core>
#include <Eigen/SVD>
#include <Eigen/QR> 
#include <Eigen/LU>

#include <cstdlib>
#include <stdexcept>
#include <fstream>

#include <list>

#include <algorithm>
#include <map>
#include <vector>
#include <gsl/gsl_sf_erf.h>

using namespace std;
using namespace dStorm;
using namespace dStorm::output;
using namespace boost::units;


namespace locprec {

const int PrecisionEstimator::Dimensions;

PrecisionEstimator::EstimationResult
PrecisionEstimator::estimate_deviation_from_initial_estimate(
   const PointSet& all_data, const SubSet& estimate )
{
#if 0
    SubSet cur_estimate = estimate;
    for (int i = 0; i < 50; ++i) {
        PointSet c = remove_outliers(all_data, cur_estimate);
        cur_estimate = SubSet(c);
    }
#endif

                  
   return estimate_deviation(all_data, compute_robust_subset(all_data));
}

std::ostream& operator<<(std::ostream& o, const PrecisionEstimator::EstimationResult& r)
{	
	Eigen::VectorXf vec = r.sd;
	vec = vec.array().sqrt();
	// fwhm
	vec *= 2.35;

	// caluclation of the main principals angle in radians
	float phi = atan2 (r.pcs.row(0)[0],r.pcs.row(0)[1]);
	o << r.center.transpose() << " " << vec.transpose() << " " << phi << "\n";
	return o;
}

PrecisionEstimator::SubSet::SubSet( const PrecisionEstimator::PointSet& subset ) 
: subset(subset)
{
		_center = center_of_mass(this->subset);
		PointSet centered = this->subset;
		center( centered );
		_cov = cov(centered);
		determinant = _cov.determinant();
}


bool PrecisionEstimator::SubSet::operator<( const SubSet& other )
	   { return determinant < other.determinant; }


bool PrecisionEstimator::SubSet::operator!=( const SubSet& other )
	   { return determinant != other.determinant; }
	

void PrecisionEstimator::SubSet::swap(SubSet& b) {
	     std::swap(subset, b.subset);
	     std::swap(_cov, b._cov);
	     std::swap(_center, b._center);
	     std::swap(determinant, b.determinant);
}


PrecisionEstimator::Point PrecisionEstimator::SubSet::center_of_mass ( 
const PrecisionEstimator::PointSet &m )
{
	return m.colwise().sum() / m.rows();
}


void PrecisionEstimator::SubSet::center(PointSet &m) {
	Eigen::VectorXf col_sums = m.colwise().sum().transpose() / m.rows();
	for(int j=0; j<m.cols(); j++)
           m.col(j).array() -= col_sums[j];
}

PrecisionEstimator::PointMatrix PrecisionEstimator::SubSet::cov (
   const PointSet &m ) 
{
	PrecisionEstimator::PointSet centered_m = m;
	center(centered_m);
	return m.transpose() * m / ( m.rows() -1 );
}

PrecisionEstimator::_Config::_Config()
: simparm::Object("SVDPrecision",
                  "Robust estimate of the localization precision per spot."),
  h_value("HValue", "Percentage of expected outliers", 0.75),
  confidence_interval("ConfidenceInterval", "Confidence interval in sigmas", 2),
  outputFile("ToFile", "Save precision info to", "_prec.txt")
{
    h_value.help = "Used by the robust SVD precision estimation as a rough apporoximation to the probability of a single localization to be an outlier.The recommended default value is set to 0.75 and the final result may have more or less localizations marked as a clustering error";
}


PrecisionEstimator::PrecisionEstimator
    ( const PrecisionEstimator::Config& config )
: OutputObject("SVDPrecisionEstimator", "Per-spot precision estimator"),
  h_value( config.h_value() ),
  printTo( "TargetFile", "Used target file", config.outputFile() )
{
    distance_threshold_sq = config.confidence_interval() * config.confidence_interval();
    variance_correction = 2.5 / gsl_sf_erf( config.confidence_interval() / sqrt(2) );
}

PrecisionEstimator* PrecisionEstimator::clone() const
    { return new PrecisionEstimator(*this); }

//using namespace Precision;

Output::AdditionalData
PrecisionEstimator::announceStormSize(const Announcement& a)
{
    used_dimensions = a.position().is_given(2,0);
    return AdditionalData().set_cluster_sources();
}


void PrecisionEstimator::receiveLocalizations( const EngineResult &er )
{
    for (EngineResult::const_iterator loc = er.begin(); loc != er.end(); ++loc) {
        assert( loc->children.is_initialized() );

        typedef std::vector<dStorm::Localization> Trace;
	const Trace& trace = *loc->children;
	int size = trace.size();
	PrecisionEstimator::PointSet m(size, Dimensions);

	int i = 0;
	// get every point of the cluster
	for(Trace::const_iterator it = trace.begin(); it != trace.end(); it++ ) {
		const dStorm::Localization& point = *it;
                for (int j = 0; j < 3; ++j)
                    m(i,j) = point.position()[j].value();
                for (int j = used_dimensions; j < 3; ++j)
                    m(i,j) = 0;
		++i;
	}

	// remove clusters to small for the fmcd calculation
	if(i >= Dimensions + 2){
		// run the FMCD and the SVD
		printTo.get_output_stream() 
                  << estimate_deviation(m, compute_robust_subset(m)) << "\n";
	}
     }
}


float PrecisionEstimator::median_absolute_distance( const SubSet& subset) 
{
	assert((subset.covariance().array() > 0).any());

	const PointMatrix inv_S = subset.covariance().inverse ();
	const PointSet& points = subset.points();
	const Point& T = subset.center_of_mass();
	int rowNum = subset.points().rows();

	float distances[rowNum];
	Point a;

	// mahalanobis distance calculation
	for ( int i = 0; i < rowNum; i++ ) {
		a = points.row ( i ).transpose () -  T ;
		float distance =  ( float ) (a.transpose ().dot ( inv_S * a ));
		distances[i] = distance;
	}

        int elements = sizeof(distances) / sizeof(distances[0]);
        std::sort(distances, distances + elements);

	float median = 0;

	if ( ( rowNum & 1 ) == 1 ) {
		median = distances[( rowNum +1 ) / 2];
	}
	else {
	        median = (distances[( (rowNum/2) +1 )] + distances[(rowNum/2)]) /2;
	}
//	std::cout << "median: " << median*median << "\n" ;
        return sqrt(median);
}


PrecisionEstimator::SubSet PrecisionEstimator::compute_robust_subset(const PointSet& data) {

	const int resultSetSize = 10;

        // initialize result array
	SubSet results[resultSetSize+1];

	gsl_rng *rng = gsl_rng_alloc( gsl_rng_taus );

	int h =  h_value*data.rows();

	for ( int i = 0; i < 500; i++ ) {

		// generate list of random numbers	
		std::vector<int> randNum;
		for ( int r = 0; r < data.rows(); r++ ) {
			randNum.push_back ( r );
		}

		// generate random p+1 subset j
		PrecisionEstimator::PointSet j_0(Dimensions+1, Dimensions);
		for ( int u = 0; u < Dimensions+1; u++ ) {

			int rand_int = gsl_rng_uniform_int( rng, randNum.size() );

			for ( int v = 0; v < Dimensions; v++ )
				j_0( u,v ) = data ( randNum[rand_int], v );

			randNum.erase ( randNum.begin() +rand_int );
		}
		PrecisionEstimator::SubSet j(j_0);

		// get S_0
		PrecisionEstimator::PointMatrix S = j.covariance();

		// add random observations while det S < 0
		while ( S.determinant() <= 0) {
			PointSet temp(j.points().rows()+1, Dimensions);
		
			int rand_int =  gsl_rng_uniform_int( rng, randNum.size() );
			temp << j.points(), data.row(randNum[rand_int]);

			randNum.erase ( randNum.begin() +rand_int );
			SubSet t = SubSet(temp);
			j = t;
			S = j.covariance();
		}

		// compute initial subset H with length h
		PrecisionEstimator::PointSet H_0 = cStep (S, j.center_of_mass(), data);
		H_0 = H_0.topLeftCorner( h, Dimensions );
		
		PrecisionEstimator::SubSet H(H_0);

		// two c-steps
		for ( int x = 0; x < 2; x++) {
			PrecisionEstimator::PointSet H_step = cStep (
			     H.covariance(), H.center_of_mass(), data).topLeftCorner( h, Dimensions);

			PrecisionEstimator::SubSet h (H_step);
			H = h;
		}

		// sort the resulting data into the result array or discard them
		int v = std::min ( i, resultSetSize );
		PrecisionEstimator::SubSet me ( H.points() );
		std::swap ( results[v], me);
		while ( v > 0 && results[v] < results[v-1] )
		{
			std::swap ( results[v], results[v-1]);
			v--;
		}
	}

	PrecisionEstimator::SubSet* chosen_subset = NULL;

// perform c-steps for the 10 lowest determinants
	for ( int i=0; i < resultSetSize; i++ ) {
		for ( int step = 0; step < 50; step++ ) {
			const PointMatrix& covMatrix = results[i].covariance();
			const Point& tVector = results[i].center_of_mass();

			PointSet hMatrix =
			    cStep ( covMatrix, tVector, data).topLeftCorner( h, Dimensions);

			PrecisionEstimator::SubSet new_subset ( hMatrix );

			if ( new_subset != results[i] )
				std::swap ( new_subset, results[i] );
			else {
				break;
			}
		}

		if ( chosen_subset == NULL || results[i] < *chosen_subset )
			chosen_subset = &results[i];
	}
	gsl_rng_free( rng );

	return *chosen_subset;
}


PrecisionEstimator::PointSet PrecisionEstimator::cStep(const PointMatrix &S, const Point &T, const PointSet &x) {
	typedef std::multimap< float, int > SortMap;
	SortMap sorting;
	Point a;

	PrecisionEstimator::PointMatrix inv_S = S.inverse ();
	PrecisionEstimator::PointSet H ( x.rows (), Dimensions);
	Eigen::VectorXf distances ( x.rows () );

	// mahalanobis distance calculation and sorting
	for ( int i = 0; i < x.rows (); i++ ) {
		a = x.row ( i ).transpose () - T;
		float distance = ( float ) sqrt ( ( float ) a.transpose ().dot ( inv_S * a ) );

		sorting.insert ( std::make_pair ( distance, i ) );
	}

	int curRow = 0;
	for ( SortMap::const_iterator i = sorting.begin(); i != sorting.end(); ++i )
	{
		H.row ( curRow ) = x.row ( i->second );
		curRow++;
	}

	return H;
}


PrecisionEstimator::PointSet PrecisionEstimator::remove_outliers(const PointSet& full_data, const SubSet& robustSet)
{
	int rows = full_data.rows();
	// center vector of the robust points
	const Point& T = robustSet.center_of_mass();

	// covariance matrix of the robust set
	const PointMatrix& covariance = robustSet.covariance();
// std::cout << "covariancematrix: \n" << covariance << "\n";

// std::cout << "scale_correction: " << scale_correction << "\n";
	assert(variance_correction > 1);
	PrecisionEstimator::PointMatrix c_covariance = variance_correction * covariance;

	PrecisionEstimator::PointMatrix inv_robustCovariance = c_covariance.inverse();

	PrecisionEstimator::PointSet sorted_points ( rows, Dimensions );
	int last_a = -1, first_b = rows;

// calculate the outlier vector
	for ( int i = 0; i < rows; i++ ) {
		Point a = full_data.row ( i ).transpose() - T;

		float distance_sq = a.transpose().dot(inv_robustCovariance*a );
		if ( distance_sq <=  distance_threshold_sq) {
			sorted_points.row ( ++last_a ) = full_data.row ( i );
		}
		else {
			sorted_points.row ( --first_b ) = full_data.row ( i );
		}
	}

	assert ( last_a + 1 == first_b );

#if 0
	int outlier_count =  rows - first_b;
	
	PointSet outliers(outlier_count);
	if ( outlier_count > 0 )
		outliers << sorted_points.block ( first_b, 0, outlier_count,
		 Dimensions );
	else	outliers << PointSet::Zero ( 1,Dimensions );

#endif

	assert(last_a > -1);

	return sorted_points.topLeftCorner( last_a+1, Dimensions);
}


PrecisionEstimator::EstimationResult PrecisionEstimator
   ::estimate_deviation(const PointSet& data, const SubSet& subset )
{

	// runs FMCD calculation and outlier removal
	PointSet gPoints = PrecisionEstimator::remove_outliers(data,subset);
	SubSet good_points(gPoints);

	// get the final robustly estimated set of localizations
	Point robustCenter = good_points.center_of_mass();
	
        /* Compute a robust covariance matrix by mormating the good
         * points to zero mean. */
	PointSet goodSub = good_points.points();
	for(int i =0; i < Dimensions; i++)
		goodSub.col(i).array() -= robustCenter(i);

	PointMatrix robustCovariance = (goodSub.transpose() * goodSub)  /
			(goodSub.rows()-1) * variance_correction;

	// run SVD on the robust covariancematrix
	Eigen::JacobiSVD<Eigen::MatrixXf> svd ( robustCovariance );

	Eigen::Matrix<float, Dimensions, 1> sd = Eigen::Matrix<float, Dimensions, 1>::Zero();
	sd<< svd.singularValues()[0],svd.singularValues()[1];

	Eigen::Matrix<float, Dimensions, 2> pc_dir;
	pc_dir << svd.matrixU();

	EstimationResult result = {robustCenter, sd, pc_dir, robustCovariance, 
	data.rows(), good_points.points()};

	return result;
}


void PrecisionEstimator::store_results() {
    printTo.close_output_stream();
}

}


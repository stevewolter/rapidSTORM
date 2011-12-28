#ifndef LOCPREC_PRECISIONESTIMATOR_H
#define LOCPREC_PRECISIONESTIMATOR_H

#include <memory>

#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/output/FileOutputBuilder.h>

#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>

#include <Eigen/Core>


namespace locprec {
class PrecisionEstimator 
    : public dStorm::output::OutputObject 
{
   public:
	static const int Dimensions = 3;
	class SubSet;
	typedef Eigen::Matrix<float, Eigen::Dynamic, Dimensions> PointSet;
   private:
        int used_dimensions;
        class _Config;
	typedef Eigen::Matrix<float, Dimensions, Dimensions> PointMatrix;
   public:
	typedef Eigen::Matrix<float, Dimensions, 1> Point;
	/* contains the result of the FMCD */
	class EstimationResult;
   private:
	/* the percentage of expected outliers in the set */
	const float h_value;

	// initialize the needed chiSquare values
	float chiSquare_500_Value;
	float distance_threshold_sq, variance_correction;

        simparm::FileEntry printTo;

	/** Calculates a concentration step which results in a PointSet,
	* contining all data points ordered by their mahalanobis distances
	* based on the center vector T and a covariance matrix S.
	*@param S the covariance matrix to use for distance calculation
	*@param T the vector pointing to the center
	*@param x PointSet to calculate
	*@return full set ordered by MH distances */
	PrecisionEstimator::PointSet cStep ( const PointMatrix& S, const Point& T,
		const PointSet& x );

	/** Draws 500 semi random subsets of size h and concentrates them using two
	* concentration steps. Concentration steps for the best 10 results are run until
	* convergence (their covariancematrix determinant does not change anymore).
	* The subset with the smallest covariance matrix determinant is returned.
	*@param data the full data set
	*@return subset with the smallest determinant */
	PrecisionEstimator::SubSet compute_robust_subset( const PointSet &data );

	float median_absolute_distance( const SubSet& subset );

	/** Removes possible outliers from the dataset. To obtain
	* consistency with normal distributed data a scale correction factor
	* which is based on the median of squared distances of the robut data is used.
	*@param full_data the full dataset
	*@param robustSet the robustly estimated subset
	*@return PointSet contining only the possible non outliers */
	PrecisionEstimator::PointSet remove_outliers(
		const PointSet& full_data, const SubSet& robustSet);

	/** Calulates the FMCD for a given cluster and the SVD which yields
	* the standard deviation of the cluster's principal components.
	* @param PointSet set of datapoints
	* @return EstimationResult */
	EstimationResult estimate_deviation(const PointSet& data, const SubSet& robust_subset );

	friend std::ostream& operator<<(std::ostream&, const PrecisionEstimator::EstimationResult&);
      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::output::FileOutputBuilder<PrecisionEstimator> Source;

	PrecisionEstimator ( const Config& config );
	PrecisionEstimator *clone() const;

        EstimationResult estimate_deviation_from_initial_estimate(
            const PointSet& all_data, const SubSet& estimate );

        AdditionalData announceStormSize(const Announcement&); 
        void receiveLocalizations(const EngineResult&);
        void store_results();

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
};

class PrecisionEstimator::_Config : public simparm::Object
{
      protected:
         void registerNamedEntries()
         {
               push_back ( h_value );
               push_back ( outputFile );
         }

      public:
         simparm::Entry<double> h_value, confidence_interval;
         dStorm::output::BasenameAdjustedFileEntry outputFile;

         _Config();
         bool can_work_with( dStorm::output::Capabilities cap )
            { return cap.test_cluster_sources() ; }
};

/** Dataclass able to hold a pointset and calculate its centervector,
* covariancematrix and their determinant. */
class PrecisionEstimator::SubSet
{
         PointSet subset;
         PointMatrix _cov;
         Point _center;
         float determinant;

         public:
         SubSet() {}
         /** Constructor for a subset containing some data. */
         SubSet (const PointSet& subset);

         /** Calculates the covariance matrix. */
         PointMatrix cov ( const PointSet &m );

         /**  Compares the covariance matrix determinants
         * of the given subsets. */
         bool operator<( const SubSet& other );
         bool operator!=( const SubSet& other );

         void swap(SubSet& b);

         const PointSet& points() const { return subset; }
         const PointMatrix& covariance() const { return _cov; }
         PointMatrix& covariance() { return _cov; }
         const Point& center_of_mass() const { return _center; }
         float getDeterminant() const { return determinant; }

         /** Centers the given PointSet around zero. */
         void center(PointSet &m);

         /** Computes the vector to the center of mass. */
         Point center_of_mass(const PointSet &m);

};

struct PrecisionEstimator::EstimationResult
{	
	/* robust center */
	Point center;
	/* lenght of the principal component vectors */
	Eigen::Matrix<float, Dimensions, 1> sd;
	/* vector pointing in the direction of the main prinicpal component */
	Eigen::Matrix<float, Dimensions, 2> pcs;
	/* robust covariancematrix */
	PrecisionEstimator::PointMatrix covariance;
	/* size of original cluster */
	int size;
	/* localizations cosidered to be non outliers */
	PointSet good_points;
};
	
}
#endif

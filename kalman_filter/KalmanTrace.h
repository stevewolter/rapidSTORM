#ifndef LOCPREC_KALMANTRACE_H
#define LOCPREC_KALMANTRACE_H

#include <Eigen/Core>
#include <Eigen/LU>
#include <boost/units/cmath.hpp>
#include <boost/units/Eigen/Array>

#include "kalman_filter/units.h"

namespace dStorm {
namespace kalman_filter {

using namespace boost::units;

struct KalmanMetaInfo {
    static const int Dimensions = 2;

    KalmanMetaInfo()
        : random_system_dynamics_covar( Eigen::Matrix<double,2*Dimensions,2*Dimensions>::Zero() ) {}

    void set_diffusion(int dim, quantity<diffusion_unit> value)
        { random_system_dynamics_covar(dim,dim) = value.value(); }
    void set_mobility(int dim, quantity<mobility_unit> value)
        { random_system_dynamics_covar(dim+Dimensions,dim+Dimensions) = value.value(); }

  private:
    friend class KalmanTrace;
    Eigen::Matrix<double,2*Dimensions,2*Dimensions> random_system_dynamics_covar;
};

class KalmanTrace {
    template <int Dim>
    struct Position {
        Eigen::Matrix<double,Dim,1> position;
        Eigen::Matrix<double,Dim,Dim> covariance;
        int time;
    };

  public:
    static const int Dimensions = 2;
    static const int State = 2*Dimensions;
    static const int Obs = Dimensions;

    typedef Position<Dimensions> Observation;

    KalmanTrace( const KalmanMetaInfo& meta );

    float sq_distance_in_sigmas(const Observation& position) ; 
    void add(const Observation& position);

    Eigen::Matrix<double,Dimensions,1> getPositionEstimate() const {
        return estimate.position.head<Dimensions>(); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  private:
    const KalmanMetaInfo& meta;

    Eigen::Matrix<double,Obs,State>    observation_matrix;

    bool have_estimate;
    Position<State> estimate;

    bool have_prediction;
    Position<State> prediction;

    Eigen::Matrix<double,State,State> state_transition(int t) const;
    Eigen::Matrix<double,State,State> system_covariance(int t) const;

    /** Update the \c position_prediction and the \c prediction_precision
     *  to be current at time \c to_prediction. Indicate this state using
     *  the \c prediction variable. */
    void update_prediction( const int to_prediction );

    /** Update the \c position_estimate and the \c estimation_precision
     *  with the measurement \c measurement_vector at the time \c time. */
    void update_position(const Observation& measurement_vector);
    
};

}
}

#endif

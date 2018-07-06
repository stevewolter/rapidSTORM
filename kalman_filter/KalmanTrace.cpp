#include "kalman_filter/KalmanTrace.h"

namespace dStorm {
namespace kalman_filter {

KalmanTrace::KalmanTrace(const KalmanMetaInfo& meta)
: meta(meta), have_estimate(false), have_prediction(false) {
    observation_matrix.setZero();
    for (int i = 0; i < Dimensions; i++) observation_matrix(i,i) = 1;
}

Eigen::Matrix<double,KalmanTrace::State,KalmanTrace::State>
KalmanTrace::state_transition(int t) const {
    Eigen::Matrix<double,State,State> result = Eigen::Matrix<double,State,State>::Identity();
    for (int i = 0; i < Dimensions; i++) {
        result(i, Dimensions+i) = t;
    }
    return result;
}

Eigen::Matrix<double,KalmanTrace::State,KalmanTrace::State>
KalmanTrace::system_covariance(int t) const {
    return t * meta.random_system_dynamics_covar;
}

void KalmanTrace::update_prediction( const int to_prediction ) {
    /* Check if prediction already current. */
    if ( have_prediction && prediction.time == to_prediction ) return;

    int time_delta = to_prediction - estimate.time;
    auto state_transition = this->state_transition(time_delta);
    auto system_covariance = this->system_covariance(time_delta);

    prediction.time = to_prediction;
    /* Predictor equation */
    prediction.position = state_transition * estimate.position;
    /* Predictor covariance matrix equation */
    prediction.covariance = 
          state_transition * estimate.covariance * 
                state_transition.transpose()
        + system_covariance;
    
    have_prediction = true;
}

void KalmanTrace::update_position(const Observation& measurement)
{
    update_prediction(measurement.time);

    /* Weight equation */
    Eigen::Matrix<double,State,Obs> weight_vector
        = prediction.covariance * observation_matrix.transpose()
          * ( measurement.covariance +
                  observation_matrix * prediction.covariance
                      * observation_matrix.transpose() )
            .inverse();

    /* Filtering equation */
    estimate.position = prediction.position + 
        weight_vector * (
            measurement.position - observation_matrix * prediction.position);

    /* Corrector equation */
    estimate.covariance = 
        ( Eigen::Matrix<double,State,State>::Identity()
          - weight_vector * observation_matrix ) 
        * prediction.covariance;

    estimate.time = measurement.time;
}

float KalmanTrace::sq_distance_in_sigmas (const Observation& observation) {
    update_prediction( observation.time );

    Eigen::Matrix<double,Obs,1> expected_observation 
        = observation_matrix * prediction.position;
    Eigen::Matrix<double,Obs,Obs> covar_of_observations
        = observation_matrix * prediction.covariance
            * observation_matrix.transpose();

    Eigen::Matrix<double,Obs,Obs> covar_of_next_measurement =
            covar_of_observations + observation.covariance;

    Eigen::Matrix<double,Obs,1> difference 
        = expected_observation - observation.position;

    return difference.dot( 
        covar_of_next_measurement.inverse() * difference );
}

void KalmanTrace::add( const Observation& obs )
{
    if ( have_estimate ) {
        update_position( obs );
    } else {
        estimate.position = observation_matrix.transpose() * obs.position;
        estimate.covariance = 
            observation_matrix.transpose() * obs.covariance
                                        * observation_matrix;
        estimate.time = obs.time;
        have_estimate = true;
    }
}

}
}

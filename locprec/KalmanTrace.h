#ifndef LOCPREC_KALMANTRACE_H
#define LOCPREC_KALMANTRACE_H

#include <dStorm/Localization.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include <dStorm/unit_matrix_operators.h>

namespace locprec {

using namespace boost::units;

extern int traceNumber;

template <int Dimensions>
class KalmanTrace : public std::vector<dStorm::Localization> {
    static const int State = 2*Dimensions;
    static const int Obs = Dimensions;

    struct Observation : public Eigen::Matrix<double,Obs,1> {
        Observation(const dStorm::Localization& l) {
            for (int i = 0; i < Dimensions; ++i)
                (*this)(i,0) = l.position()[i] / si::metre;
        }
    };

    Eigen::Matrix<double,State,State>  state_transition;
    Eigen::Matrix<double,Obs,State>    observation_matrix;

    const Eigen::Matrix<double,Obs,Obs>* measurement_covar;
    const Eigen::Matrix<double,State,State>* random_system_dynamics_covar;

    int position;
    Eigen::Matrix<double,State,1>      position_estimate;
    Eigen::Matrix<double,State,State>  estimation_precision;

    bool have_prediction;
    int prediction;
    Eigen::Matrix<double,State,1>      position_prediction;
    Eigen::Matrix<double,State,State>  prediction_precision;

    /** Set the \c state_transition matrix to the correct time
     *  difference. */
    void set_time_in_state_transition(const int t);

    /** Update the \c position_prediction and the \c prediction_precision
     *  to be current at time \c to_prediction. Indicate this state using
     *  the \c prediction variable. */
    void update_prediction( const int to_prediction );

    /** Update the \c position_estimate and the \c estimation_precision
     *  with the measurement \c measurement_vector at the time \c time. */
    void update_position( const Observation& measurement_vector,
                          const int time );
    
    int myNumber;

  public:
    /** Dummy constructor for dummy tracing objects. Will not produce
     *  useful tracers. */
    KalmanTrace() : measurement_covar(NULL), 
                            random_system_dynamics_covar(NULL) {}

    KalmanTrace(
        const Eigen::Matrix<double,Obs,Obs>& measurement_covar,
        const Eigen::Matrix<double,State,State>&
                    random_system_dynamics_covar
    );

    float sq_distance_in_sigmas( const dStorm::Localization& position )
; 
    void add( const dStorm::Localization& l );
    void clear();

    int getPosition() const { return position; }
    Eigen::Matrix<double,2,1> getPositionEstimate() const { return position_estimate.template start<2>(); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <int Dimensions>
KalmanTrace<Dimensions>::KalmanTrace(
    const Eigen::Matrix<double,Obs,Obs>& measurement_covar,
    const Eigen::Matrix<double,State,State>& random_system_dynamics_covar
)
: measurement_covar( &measurement_covar ),
  random_system_dynamics_covar( &random_system_dynamics_covar )
{
    state_transition.setIdentity();
    observation_matrix.setZero();
    for (int i = 0; i < Dimensions; i++) observation_matrix(i,i) = 1;

    clear();
}

template <int Dimensions>
void KalmanTrace<Dimensions>::set_time_in_state_transition(const int t)
 
{
    for (int i = 0; i < Dimensions; i++) {
        state_transition(i, Dimensions+i) = t;
    }
}

template <int Dimensions>
void KalmanTrace<Dimensions>::update_prediction( const int to_prediction )
 
{
    /* Check if prediction already current. */
    if ( have_prediction && prediction == to_prediction ) return;

    prediction = to_prediction;
    set_time_in_state_transition( prediction - position );

    /* Predictor equation */
    position_prediction = state_transition * position_estimate;
    /* Predictor covariance matrix equation */
    prediction_precision = 
          state_transition * estimation_precision * 
                state_transition.transpose()
        + (*random_system_dynamics_covar);
    
    have_prediction = true;
}

template <int Dimensions>
void KalmanTrace<Dimensions>::update_position( 
    const Observation& measurement_vector,
    const int time)
{
    if ( !have_prediction || time != prediction ) update_prediction(time);

    /* Weight equation */
    Eigen::Matrix<double,State,Obs> weight_vector
        = prediction_precision * observation_matrix.transpose()
          * ( *measurement_covar +
                  observation_matrix * prediction_precision
                      * observation_matrix.transpose() )
            .inverse();

    /* Filtering equation */
    position_estimate = position_prediction + 
        weight_vector * (
            measurement_vector - observation_matrix * position_prediction);

    /* Corrector equation */
    estimation_precision = 
        ( Eigen::Matrix<double,State,State>::Identity()
          - weight_vector * observation_matrix ) 
        * prediction_precision;

    position = time;
}

template <int Dimensions>
float KalmanTrace<Dimensions>::sq_distance_in_sigmas ( 
    const dStorm::Localization& position 
)
{
    update_prediction( position.frame_number() / camera::frame );

    Eigen::Matrix<double,Obs,1> expected_observation 
        = observation_matrix * position_prediction;
    Eigen::Matrix<double,Obs,Obs> covar_of_observations
        = observation_matrix * prediction_precision 
            * observation_matrix.transpose();

    Eigen::Matrix<double,Obs,Obs> covar_of_next_measurement =
            covar_of_observations + *measurement_covar;

    Observation real_observation(position);

    Eigen::Matrix<double,Obs,1> difference 
        = expected_observation - real_observation;

    double sq_dist = difference.dot( 
        covar_of_next_measurement.inverse() * difference );
#if 0
    std::cout << "0 " 
        << myNumber << " "
        << position.getImageNumber() << " "
        << size() << " "
        << expected_observation.transpose() << " "
        << real_observation.transpose() << " "
        << covar_of_next_measurement.diagonal().transpose() << " "
        << sq_dist << "\n";
#endif
    return sq_dist;
}

template <int Dimensions>
void KalmanTrace<Dimensions>::add( const dStorm::Localization& l )
{
    Observation obs(l);
    bool previous_observations_available = (size() > 0);
    if ( previous_observations_available )
        update_position( obs, l.frame_number() / camera::frame );
    else {
        position_estimate = observation_matrix.transpose() * obs;
        position = l.frame_number() / camera::frame;
    }
#if 0
    std::cout << "1 " 
        << myNumber << " "
        << l.getImageNumber() << " "
        << size() << " "
        << obs.transpose() << " "
        << position_estimate.transpose() << " "
        << estimation_precision.diagonal().transpose() << " for "
        << this << "\n";
#endif
    this->push_back( l );
}

template <int Dimensions>
void KalmanTrace<Dimensions>::clear() 
{
    estimation_precision = 
        observation_matrix.transpose() * (*measurement_covar)
                                       * observation_matrix;
    for (int i = Dimensions; i < 2*Dimensions; i++)
        estimation_precision(i,i) = 0;
    have_prediction = false;
    std::vector<dStorm::Localization>::clear();

    myNumber = traceNumber++;
}

};

#endif

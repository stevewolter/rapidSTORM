#ifndef LOCPREC_KALMANTRACE_H
#define LOCPREC_KALMANTRACE_H

#include <dStorm/Localization.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include <boost/units/Eigen/Array>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/camera/time.hpp>

namespace dStorm {
namespace kalman_filter {

using namespace boost::units;

template <int Dimensions> class KalmanTrace;

template <int Dimensions>
struct KalmanMetaInfo {
    typedef boost::units::si::length observation_unit;
    typedef boost::units::multiply_typeof_helper<observation_unit,observation_unit>::type obs_variance_unit;
    typedef boost::units::camera::time time_unit;
    typedef boost::units::divide_typeof_helper< observation_unit, time_unit >::type speed_unit;
    typedef boost::units::divide_typeof_helper< obs_variance_unit, time_unit >::type diffusion_unit;
    typedef boost::units::divide_typeof_helper< 
        boost::units::multiply_typeof_helper<speed_unit,speed_unit>::type,
        time_unit >::type mobility_unit;

    KalmanMetaInfo()
        : random_system_dynamics_covar( Eigen::Matrix<double,2*Dimensions,2*Dimensions>::Zero() ) {}

    void set_diffusion(int dim, quantity<diffusion_unit> value)
        { random_system_dynamics_covar(dim,dim) = value.value(); }
    void set_mobility(int dim, quantity<mobility_unit> value)
        { random_system_dynamics_covar(dim+Dimensions,dim+Dimensions) = value.value(); }

  private:
    friend class KalmanTrace<Dimensions>;
    Eigen::Matrix<double,2*Dimensions,2*Dimensions> random_system_dynamics_covar;
};

template <int Dimensions>
class KalmanTrace : public std::vector<dStorm::Localization> {
    static const int State = 2*Dimensions;
    static const int Obs = Dimensions;

    struct Observation {
        Eigen::Matrix<double,Obs,1> position;
        Eigen::Matrix<double,Obs,Obs> uncertainty;
        Observation(const dStorm::Localization& l) {
            uncertainty.fill(0);
            for (int i = 0; i < Dimensions; ++i) {
                position[i] = l.position()[i] / si::metre;
                uncertainty(i,i) = pow<2>(l.position_uncertainty()[i] / si::metre);
            }
        }
    };

    const KalmanMetaInfo<Dimensions>& meta;

    Eigen::Matrix<double,State,State>  state_transition;
    Eigen::Matrix<double,Obs,State>    observation_matrix;

    Eigen::Matrix<double,State,State> system_covar;

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
    
  public:
    KalmanTrace( const KalmanMetaInfo<Dimensions>& meta );

    float sq_distance_in_sigmas( const dStorm::Localization& position ) ; 
    void add( const dStorm::Localization& l );
    void clear();

    int getPosition() const { return position; }
    Eigen::Matrix<double,2,1> getPositionEstimate() const { return position_estimate.template head<2>(); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <int Dimensions>
KalmanTrace<Dimensions>::KalmanTrace(
    const KalmanMetaInfo<Dimensions>& meta
)
: meta(meta)
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
    system_covar = t * meta.random_system_dynamics_covar;
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
        + system_covar;
    
    have_prediction = true;
}

template <int Dimensions>
void KalmanTrace<Dimensions>::update_position( 
    const Observation& measurement,
    const int time)
{
    if ( !have_prediction || time != prediction ) update_prediction(time);

    /* Weight equation */
    Eigen::Matrix<double,State,Obs> weight_vector
        = prediction_precision * observation_matrix.transpose()
          * ( measurement.uncertainty +
                  observation_matrix * prediction_precision
                      * observation_matrix.transpose() )
            .inverse();

    /* Filtering equation */
    position_estimate = position_prediction + 
        weight_vector * (
            measurement.position - observation_matrix * position_prediction);

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

    Observation observation(position);

    Eigen::Matrix<double,Obs,Obs> covar_of_next_measurement =
            covar_of_observations + observation.uncertainty;

    Eigen::Matrix<double,Obs,1> difference 
        = expected_observation - observation.position;

    double sq_dist = difference.dot( 
        covar_of_next_measurement.inverse() * difference );
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
        position_estimate = observation_matrix.transpose() * obs.position;
        position = l.frame_number() / camera::frame;
        estimation_precision = 
            observation_matrix.transpose() * obs.uncertainty
                                        * observation_matrix;
        for (int i = Dimensions; i < 2*Dimensions; i++)
            estimation_precision(i,i) = 0;
    }
    this->push_back( l );
}

template <int Dimensions>
void KalmanTrace<Dimensions>::clear() 
{
    have_prediction = false;
    std::vector<dStorm::Localization>::clear();
}

}
}

#endif

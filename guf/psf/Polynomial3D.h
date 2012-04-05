#ifndef DSTORM_PSF_ZHUANG_H
#define DSTORM_PSF_ZHUANG_H

#include "Base3D.h"
#include <nonlinfit/append.h>
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/DerivationSummand.h>
#include <nonlinfit/DerivationSummand.h>
#include <boost/mpl/iter_fold.hpp>
#include <boost/mpl/range_c.hpp>
#include <dStorm/polynomial_3d.h>
#include <dStorm/Direction.h>
#include <boost/optional/optional.hpp>

namespace dStorm {
namespace guf {
namespace PSF {

class Polynomial3D
: public Base3D,
  public nonlinfit::access_parameters<Polynomial3D>
{
  public:
    static const int Order = polynomial_3d::Order;
  private:
    template <typename Type> friend class nonlinfit::access_parameters;
    template <class Num, typename Expression> friend class Parameters;
    template <class Num, typename Expression, int Size> friend class JointEvaluator;
    template <class Num, typename Expression, int Size> friend class DisjointEvaluator;

    Eigen::Array2d best_sigma;
    Eigen::Vector2d zposition;
    Eigen::Array< double, 2, Order+1 > delta_sigma;

    template <int Index> double& access( BestSigma<Index> ) { return best_sigma[Index]; }
    template <int Index> double& access( ZPosition<Index> ) { return zposition[Index]; }
    template <int Index, int Term> double& access( DeltaSigma<Index,Term> ) { return delta_sigma(Index,Term); }
    using Base3D::access;

    template <class Sequence, class Number>
    struct add_delta_sigmas {
        typedef typename boost::mpl::push_back< 
            typename boost::mpl::push_back< 
                Sequence, 
                DeltaSigma<Direction_X,Number::value> >::type,
            DeltaSigma<Direction_Y,Number::value> 
        >::type type;
    };

    typedef typename nonlinfit::append< 
            typename Base3D::Variables,
            boost::mpl::vector< BestSigma<0>, BestSigma<1>, ZPosition<0>, ZPosition<1> >
        >::type BaseVariables;

  public:
    /* The variables in Polynomial3D are the DeltaSigma in X and Y for each 
     * power term. */
    typedef typename boost::mpl::fold<
        boost::mpl::range_c<int,polynomial_3d::FirstTerm,polynomial_3d::LastTerm+1>,
        BaseVariables,
        boost::mpl::quote2<add_delta_sigmas>
    >::type Variables;

    Polynomial3D& copy( const BaseExpression& f ) { return *this = dynamic_cast<const Polynomial3D&>(f); }

    Eigen::Matrix< quantity<MeanZ::Unit>, 2, 1 > get_sigma() const;

    boost::units::quantity< Micrometers > get_delta_sigma
        ( int dimension, int term ) const;

    using nonlinfit::access_parameters<Polynomial3D>::operator();
    using nonlinfit::access_parameters<Polynomial3D>::get;
};

template <typename Num>
class Parameters< Num, Polynomial3D >
: public BaseParameters<Num>
{
    boost::optional< Eigen::Array<Num,2,1> > compute_sigma_();
    void compute_prefactors_();

  protected:
    const Polynomial3D* expr;
    Eigen::Array<Num,2,1> z_deriv_prefactor, relative_z, threed_factor;
    Eigen::Array<Num,2,polynomial_3d::Order+1> delta_z_deriv_prefactor;
  public:
    Parameters() {}
    Parameters( const Polynomial3D& expr ) : BaseParameters<Num>(expr), expr(&expr) {}
};

}
}
}

#endif

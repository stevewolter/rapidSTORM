#ifndef DSTORM_MSD_PSF_EVALUATOR_H
#define DSTORM_MSD_PSF_EVALUATOR_H


#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/Xs.h>
#include "parameters.h"
#include <nonlinfit/Evaluator.h>

namespace dStorm {
namespace measured_psf {

template <typename Num, int ChunkSize>
struct Evaluator
{
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;

    const Model * expr;
    Eigen::Array<double,ChunkSize, 3> calib_image_pos_in_px;

    JointEvaluator() : expr(NULL)  {}
    JointEvaluator(const Model& e ) :expr(&e) {}
    bool prepare_iteration( const Data& data ) {
        return true;
    }

    void prepare_chunk( const Eigen::Array<Num,ChunkSize,2>& xs ) // change to   Eigen::Array<Num,ChunkSize,3>& xs  ???
    {
        Eigen::Array<Num, 1, 3> subpixel_in_um;
        for (int row = 0; row < ChunkSize; ++row) {
             x.head<2>() = xs.row(row);
             x[2]= expr-> z_pos;
             calib_image_pos_in_um = (x - expr->x0) + expr->image_x0; //returns relative coordinates for x_image in x_psf
             result[row]= get_psf_value(subpixel_in_um);
        }
    }

    Num get_psf_value(Eigen::Array<Num, 1, 3>)
    {
        return Num =0;
    }

    void value( Eigen::Array<Num,ChunkSize,1>& result ) { // return value_psf
//            result[row] = this->expT;
            result[row] = get_point_value(subpixel_in_um[row]);
    }
    void add_value( Eigen::Array<Num,ChunkSize,1>& result )
        { result += this->expT; }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
        target = (expT * normed.col(Dim)) * -this->sigmaI[Dim];
    }
    template <typename Target, int Dim>
    void derivative( Target target, Mean<Dim> ) {
        target = (expT * normed.col(Dim)) * this->sigmaI[Dim];
    }

    template <typename Target>
    void derivative( Target target, const Amplitude& ) {
        target = Eigen::Matrix<double,ChunkSize,1>::Zero().cast<Num>();
    }

    template <typename Target>
    void derivative( Target target, const Prefactor& ) {
        target = expT / this->transmission;
    }
};

}
}

namespace nonlinfit {

struct get_evaluator<
    Model,
    plane::Joint<Num,ChunkSize,dStorm::gaussian_psf::XPosition,dStorm::gaussian_psf::YPosition> >
{
    typedef Evaluator<Num,ChunkSize> type;
};

}

#endif

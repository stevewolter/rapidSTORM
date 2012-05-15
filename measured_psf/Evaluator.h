#ifndef DSTORM_MSD_PSF_EVALUATOR_H
#define DSTORM_MSD_PSF_EVALUATOR_H


#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/Xs.h>
#include "parameters.h"
#include <nonlinfit/Evaluator.h>
#include <cmath>
#include <Eigen/Eigen>
#include <functional>

namespace dStorm {
namespace measured_psf {

template <typename Num, int ChunkSize>
struct Evaluator
{
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;

    const Model * expr;
    Eigen::Array<double,ChunkSize, 3> calib_image_pos_in_px; //chunk with postiions
    Eigen::Array<int,ChunkSize, 3> base_pos_in_px; //chunk with postiions
    Eigen::Array<Num, ChunkSize, 3> psf_data_size;

    Evaluator() : expr(NULL)  {}
    Evaluator(const Model& e ) :expr(&e) {}
    bool prepare_iteration( const Data& data ) {
        return true;
    }

    void prepare_chunk( const Eigen::Array<Num,ChunkSize,2>& xs )
    {
        psf_data_size.rowwise() = boost::units::value( expr->psf_data.sizes() ).cast<Num>() - Num(1.0);
        Eigen::Array<Num, 1, 3> subpixel_in_um;
	Eigen::Array3d calib_image_pos_in_um;
        for (int row = 0; row < ChunkSize; ++row) {
             Eigen::Vector3d x;
             x.head<2>() = xs.row(row);
             x[2]= expr->axial_mean;
             //returns relative coordinates for x_image in x_psf
             calib_image_pos_in_um = (x - expr->x0) + expr->image_x0; 
	     //position in psf, need to get value from here
             calib_image_pos_in_px.row(row) = calib_image_pos_in_um.array() / expr->pixel_size.array(); 
        }
        if ( (calib_image_pos_in_px >= psf_data_size).any() )
            throw std::logic_error ("calib_image_pos out of range of PSF");
        base_pos_in_px = calib_image_pos_in_px.unaryExpr (std::ptr_fun (floor)).template cast<int>();
    }

    void value( Eigen::Array<Num,ChunkSize,1>& ref ) { ref.fill(0); add_value(ref); }
    void add_value( Eigen::Array<Num,ChunkSize,1>& ref ) {
        for ( int dx = 0; dx < 2; ++dx )
            for ( int dy = 0; dy < 2; ++dy )
                for ( int dz = 0; dz < 2; ++dz )
                {
                    for (int row = 0; row < ChunkSize; ++row) {
                        Eigen::Array3i pos;
                        pos.x() = base_pos_in_px.row(row).x() + dx;
                        pos.y() = base_pos_in_px.row(row).y() + dy;
                        pos.z() = base_pos_in_px.row(row).z() + dz;

                        double value = expr->psf_data( from_value<camera::length>( pos ) );
                        double weight = ( 1 - ( pos.cast<double>() - calib_image_pos_in_px.row(row).transpose() ).abs() ).prod();
                        ref[row] += weight * value;
                    }
                }
    }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
        target.fill(0);
        for ( int dx = 0; dx < 2; ++dx )
            for ( int dy = 0; dy < 2; ++dy )
                for ( int dz = 0; dz < 2; ++dz )
                {
                    for (int row = 0; row < ChunkSize; ++row) {
                        Eigen::Array3i pos;
                        pos.x() = base_pos_in_px.row(row).x() + dx;
                        pos.y() = base_pos_in_px.row(row).y() + dy;
                        pos.z() = base_pos_in_px.row(row).z() + dz;

                        double value = expr->psf_data( from_value<camera::length>( pos ) );
                        Eigen::Vector3d weights = 1 - ( pos.cast<double>() - calib_image_pos_in_px.row(row).transpose() ).abs();
                        weights[Dim] = ( pos[Dim] < calib_image_pos_in_px(row,Dim) ) ? -1 : 1;
                        target[row] += weights.prod() * value;
                    }
                }
    }
    template <typename Target, int Dim>
    void derivative( Target target, Mean<Dim> ) {
        target = Eigen::Matrix<double,ChunkSize,1>::Zero().template cast<Num>();
    }

    template <typename Target>
    void derivative( Target target, const Amplitude& ) {
        target = Eigen::Matrix<double,ChunkSize,1>::Zero().template cast<Num>();
    }

    template <typename Target>
    void derivative( Target target, const Prefactor& ) {
        target = Eigen::Matrix<double,ChunkSize,1>::Zero().template cast<Num>();
    }
};

}
}

namespace nonlinfit {

template <typename Num, int ChunkSize>
struct get_evaluator<
    dStorm::measured_psf::Model,
    plane::Joint<Num,ChunkSize,dStorm::measured_psf::XPosition,dStorm::measured_psf::YPosition> >
{
    typedef dStorm::measured_psf::Evaluator<Num,ChunkSize> type;
};

}

#endif

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
    Eigen::Array<Num, ChunkSize, 1> result;
    Evaluator() : expr(NULL)  {}
    Evaluator(const Model& e ) :expr(&e) {}
    bool prepare_iteration( const Data& data ) {
        return true;
    }

    void prepare_chunk( const Eigen::Array<Num,ChunkSize,2>& xs )
    {
        Eigen::Array<Num, 1, 3> subpixel_in_um;
	Eigen::Array3d calib_image_pos_in_um;
        for (int row = 0; row < ChunkSize; ++row) {
             Eigen::Vector3d x;
             x.head<2>() = xs.row(row);
             x[2]= expr->axial_mean;
             calib_image_pos_in_um = (x - expr->x0) + expr->image_x0; //returns relative coordinates for x_image in x_psf
             calib_image_pos_in_px.row(row) = calib_image_pos_in_um.array() / expr->pixel_size.array(); //position in psf, need to get value from here
             get_psf_value();
        }
    }

    void get_psf_value()
    {
      double delta =0;
	  for (int row = 0; row < ChunkSize; ++row)
	 {
        if (calib_image_pos_in_px.row(row).maxCoeff() > 10) throw std::logic_error("calib_image_pos out of range of PSF"); //(double) expr->psf_data.size() doesn't work
        else
        {
          Eigen::Array3d tmp= calib_image_pos_in_px.row(row).unaryExpr(std::ptr_fun(floor));
        Eigen::Array3i pixel_pos = tmp.cast<int>(); //base().cast<double>() unaryExpr(std::ptr_fun(floor))
        Image<double,3>::Position pos = from_value< camera::length >( pixel_pos );
        double psf_value_no_interpol = expr->psf_data( pos );

        for (int dim=0; dim<2; dim++)
         {
             bool is_on_right_border = false;
             bool is_on_left_border = false;
             double  remainder = floor((double)calib_image_pos_in_px(dim,row));
             if (calib_image_pos_in_px(dim,row) <0.5) is_on_left_border=true;
             if (calib_image_pos_in_px(dim,row) >= 9.5) is_on_right_border=true; //data.size() ok?? -1.5??

             if  ((( (double)calib_image_pos_in_px(dim,row) - remainder)  <0.5 || is_on_right_border ) && !is_on_left_border) //if in left half of pixel or on right border
             {
              Eigen::Array3i pixel_pos_next_px = pixel_pos;
              pixel_pos_next_px[dim]-=1;
              Image<double,3>::Position pos_next_px = from_value< camera::length >(pixel_pos_next_px);
                delta+=(expr->psf_data( pos_next_px)-expr->psf_data(pos))*remainder;
             }
             else
             {
              Eigen::Array3i pixel_pos_next_px = pixel_pos; //if in right half of pixel or on left border
              pixel_pos_next_px[dim]+=1;
              Image<double,3>::Position pos_next_px = from_value< camera::length >(pixel_pos_next_px);
                delta+=(expr->psf_data( pos_next_px)-expr->psf_data(pos)) *remainder ;
             }

       	 }
          result[row]= psf_value_no_interpol + delta;
    }
    }
    }

         void value( Eigen::Array<Num,ChunkSize,1>& ref )
        { ref = this->result; }
    void add_value( Eigen::Array<Num,ChunkSize,1>& ref )
        { ref += this->result; }


//    void value( Eigen::Array<Num,ChunkSize,1>& result ) { // return value_psf
//	result.fill(1E-10);
//    }
//    void add_value( Eigen::Array<Num,ChunkSize,1>& result )
//        { result.fill(0); }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
        target = Eigen::Matrix<double,ChunkSize,1>::Zero().template cast<Num>();
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

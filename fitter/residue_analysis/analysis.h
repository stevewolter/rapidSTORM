#ifndef GAUSSFITTER_RESIDUE_ANALYSIS_H
#define GAUSSFITTER_RESIDUE_ANALYSIS_H

#include "fitter/FixedSized_impl.h"

template <typename Ty>
Ty sq(const Ty& a) { return a*a; }

namespace dStorm {
namespace fitter {
namespace residue_analysis {

template <class BaseFitter, int Width, int Height>
typename SizedFitter<BaseFitter,Width,Height>::SpotState
SizedFitter<BaseFitter,Width,Height>
::residue_analysis(Eigen::Vector2i* direction, int xl, int yl)
{
    typedef typename BaseFitter::OneKernel::SizeInvariants Base1;

    const int b = 0;
    const int NotDiagonal = 0, Diagonal = 1,
              RightHalf = 1, LeftHalf = 0,
              UpperHalf = 0, LowerHalf = 1,
              UpperRight = 0, LowerLeft = 1,
              UpperLeft = 0, LowerRight = 1;
    const int Horizontal = 2, Vertical = 3, 
              MainDiagonal = 0, OffDiagonal = 1;

  int worst_main_axis; double max_normed_diff = 0;
  for (unsigned int i = 0; i < this->deriver.getPosition().residues.size(); ++i) {
    const Eigen::Matrix<double,Height,Width>& R 
        = this->deriver.getPosition().residues[i];
    int xc, yc;
    common.get_center(normal.getPosition().parameters, xc, yc);
    xc -= xl; yc -= yl;
    
    Eigen::Matrix2d quadrant_sets[2];
    quadrant_sets[0].fill(0);
    quadrant_sets[1].fill(0);
    for (int col_i = R.cols()-b-1; col_i >= b; col_i--)
        for (int row_i = R.rows()-b-1; row_i >= b; row_i--) {
            const int row = row_i - yc, col = col_i - xc;
            if ( row == 0 && col == 0 ) continue;

            /* Indices to the quadrant sets. Each of these variables is
             * set to 0 or 1, depending on the side the pixel is on. */
            const int
                horiz_half = (row < 0) ? LeftHalf : RightHalf,
                vert_half = (col < 0) ? UpperHalf : LowerHalf,
                main_diag_side = (col > row) ? UpperRight : LowerLeft,
                off_diag_side = (col + row > 0) ? LowerRight : UpperLeft;

            double v = R(row_i,col_i);
            if ( row != 0 && col != 0 )
                quadrant_sets[NotDiagonal]( vert_half, horiz_half ) += v;
            if ( col != row && col + row != 0 )
                quadrant_sets[Diagonal]( main_diag_side, off_diag_side )
                    += v;
        }

    Eigen::Vector4d diag_diff;
    Eigen::Vector2d inverser(1,-1);

    diag_diff[ MainDiagonal ] = 
        inverser.dot( quadrant_sets[ NotDiagonal ] * inverser );
    diag_diff[ OffDiagonal ] = - diag_diff[ MainDiagonal ];
    diag_diff[ Vertical ] = 
        inverser.dot( quadrant_sets[ Diagonal ] * inverser );
    diag_diff[ Horizontal ] = - diag_diff[ Vertical ];

    int main_axis;
    double normed_max_diff = diag_diff.maxCoeff( &main_axis )
            / R.cwise().abs().sum();
    if ( normed_max_diff > max_normed_diff ) {
        max_normed_diff = normed_max_diff;
        worst_main_axis = main_axis;
    }
  }

    bool residue_analysis_positive = 
        ( max_normed_diff > this->common.asymmetry_threshold );
    if ( residue_analysis_positive ) {
        direction->x() =
            ( worst_main_axis == Vertical ) ? 0 :
            ( worst_main_axis == OffDiagonal ) ? -1 : 1;
        direction->y() = ( worst_main_axis == Horizontal ) ? 0 : 1;

        return Fishy;
    } else
        return Single;

}

template <class BaseFitter, int Width, int Height>
float
SizedFitter<BaseFitter,Width,Height>
::double_fit_analysis( 
    const engine::Image& image, const Eigen::Vector2i& direction, int oxl, int oyl )
{
    typedef typename BaseFitter::OneKernel::SizeInvariants Base1;
    typedef typename BaseFitter::TwoKernel::SizeInvariants Base2;
#if 0
    const int DoWi = a.residues.cols() + 2, 
              DoHe = a.residues.rows() + 2;
    const int xl = std::min<int>( std::max(0, oxl - 1), image.width-DoWi ),
              yl = std::min<int>( std::max(0, oyl - 1), image.height-DoHe );
#else
    const int xl = oxl, yl = oyl;
#endif

    this->deriver.setData( 
        image.ptr(),
        image.width() / camera::pixel,
        image.height() / camera::pixel,
        image.depth_in_pixels() );
    this->deriver.setUpperLeftCorner( xl, yl );

    float half_dist = 1.8;
    this->common.start_from_splitted_single_fit( 
        normal.getPosition().parameters, 
        &this->Base::deriver.getVariables(), direction * half_dist );

    fitpp::FitResult res 
        = this->deriver.fit( this->common.Base2::fit_function );
    if ( res != fitpp::FitSuccess )
        return 1.0;

    /* Check whether the two peaks are too far from each other to be of
     * influence. */
    if ( this->common.peak_distance_small(&this->deriver.getVariables()) )
        return std::min(1.0, this->deriver.getPosition().chi_sq / normal.getPosition().chi_sq);
    else
        return 1.0;
}


}
}
}

#endif

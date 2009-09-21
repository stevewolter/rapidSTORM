#ifndef GAUSSFITTER_RESIDUE_ANALYSIS_H
#define GAUSSFITTER_RESIDUE_ANALYSIS_H

#include "engine/GaussFitter_Specialized.h"

namespace dStorm {

template <bool FS, bool Corr, int Width, int Height>
typename SpecializedGaussFitter<FS, true, Corr, Width, Height>::SpotState
SpecializedGaussFitter<FS, true, Corr, Width, Height>
::residue_analysis(Eigen::Vector2i* direction, int xl, int yl)
{
    const int b = 0;
    const int NotDiagonal = 0, Diagonal = 1,
              RightHalf = 1, LeftHalf = 0,
              UpperHalf = 0, LowerHalf = 1,
              UpperRight = 0, LowerLeft = 1,
              UpperLeft = 0, LowerRight = 1;

    const Eigen::Matrix<double,Height,Width>& R 
        = this->c->residues;
    const int xc = round(common.Width_Invariants<FS,false>::params
                               .template getMeanX<0>()) - xl,
              yc = round(common.Width_Invariants<FS,false>::params
                               .template getMeanY<0>()) - yl;

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

    const int Horizontal = 2, Vertical = 3, 
              MainDiagonal = 0, OffDiagonal = 1;
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

    bool residue_analysis_positive = ( normed_max_diff > 
                                       common.asymmetry_threshold );
    if ( residue_analysis_positive ) {
        direction->x() =
            ( main_axis == Vertical ) ? 0 :
            ( main_axis == OffDiagonal ) ? -1 : 1;
        direction->y() = ( main_axis == Horizontal ) ? 0 : 1;

        return Fishy;
    } else
        return Single;

}

template <bool FS, bool Corr, int Width, int Height>
typename SpecializedGaussFitter<FS, true, Corr, Width, Height>::SpotState
SpecializedGaussFitter<FS, true, Corr, Width, Height>
::double_fit_analysis( 
    const Image& image, const Eigen::Vector2i& direction, int oxl, int oyl )
{
    const int DoWi = a.residues.cols(), 
              DoHe = a.residues.rows();
    const int xl = std::min<int>( std::max(0, oxl - 1), image.width-DoWi ),
              yl = std::min<int>( std::max(0, oyl - 1), image.height-DoHe );

    deriver.setData( image.ptr(), image.width, image.height );
    deriver.setUpperLeftCorner( xl, yl );

    common.start_from_splitted_single_fit( &a.parameters, direction );

    std::pair<FitResult,typename Deriver::Position*>
        fit_result = common.fit_function.fit(
            a, b, common.constants, deriver );

    fitpp::FitResult res = fit_result.first;
    if ( res != FitSuccess )
        return Single;

    int x_shift = oxl - xl, y_shift = oyl - yl;
    double new_residues = fit_result.second->residues
        .block( y_shift, x_shift, 
                this->c->residues.rows(), this->c->residues.cols()
              ).cwise().square().sum();
    
    double ratio = new_residues / this->c->chi_sq;
    if ( ratio < common.residue_threshold )
        return Double;
    else
        return Single;
}

#if 0
            double_params.change_variable_set(
                &double_fit_result.second->parameters );
            //std::cerr << "Original:\n" 
                        //<< fitResult.second->parameters << "\n"
                        //<< fitResult.second->chi_sq << "\n"
                        //<< fitResult.second->residues << "\n";
            //std::cerr << "Double-fit:\n"
                        //<< double_fitter.getPosition().parameters << "\n"
                        //<< double_fitter.getPosition().chi_sq << "\n"
                        //<< double_fitter.getPosition().residues << "\n"
                        //<< "\n";
            Eigen::Vector2d position[2];
            position[0].x() =
                double_params.template getMeanX<0>();
            position[1].x() =
                double_params.template getMeanX<1>();
            position[0].y() =
                double_params.template getMeanY<0>();
            position[1].y() =
                double_params.template getMeanY<1>();

            Eigen::Vector2d amplitudes;
            amplitudes[0] =
                double_params.template getAmplitude<0>();
            amplitudes[1] =
                double_params.template getAmplitude<1>();

            df_distance = (position[0] - position[1]).squaredNorm();
            std::cout << setprecision(6) << imNumber << " " << x_end << " " << y_end << " " << amp << " " << fitResult.second->chi_sq << " " << double_fit_result.second->residues.block( 1, 1, fitResult.second->residues.rows(), fitResult.second->residues.cols() ).cwise().square().sum() << " " << df_distance << " " << double_fit_result.second->parameters.transpose() << " " << df.strength << " " << df.dir.transpose() << "\n";
#if 0
            if ( df_distance > 0.25 ) 
            {
                if ( (amplitudes.cwise() >= common.amplitude_threshold)
                    .all() /*&& amplitudes.sum() > amp*/ )
                {
                    //for (int i = 0; i < 2; i++)
                        //new(target+i) Localization( 
                            //position[i].x(), position[i].y(), imNumber,
                            //amplitudes[i]);
                    //std::cerr << "Made two localizations " << position[0].transpose() << " (" << amplitudes[0] << ") and " << position[1].transpose() << " (" << amplitudes[1] << ") instead of " << x_end << " " << y_end << " (" << amp << ") in image " << imNumber << "\n. Residues changed from " << fitResult.second->chi_sq / (width*height) << " to " << double_fitter.getPosition().chi_sq / ((width+2)*(height+2)) << "\n" ;
                    return 0;
                } /*else {
                    int greater;
                    amplitudes.maxCoeff( &greater );
                    if ( amplitudes[greater] > amp ) {
                        x_end = position[greater].x();
                        y_end = position[greater].y();
                        amp = amplitudes[greater];
                    }
                }*/
            }
#endif
        }
        //return 0;
#endif

}

#endif

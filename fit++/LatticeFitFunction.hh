#ifndef LIBFITPP_LATTICEFITFUNCTION_H
#define LIBFITPP_LATTICEFITFUNCTION_H

#include <fit++/FitFunction.hh>
#include <cassert>
#include <algorithm>

namespace fitpp {
    template <int VarCount, int Width, int Height>
    struct LatticePosition
    : public Position<VarCount>
    {
        typedef Eigen::Matrix<double,Height,Width>
            ResidueType;
        ResidueType residues;

        void resize(int width, int height)
            { residues.resize(height,width); }
    };

    template <typename DataPoint>
    class BaseLattice {
      public:
        typedef 
            Eigen::Matrix<DataPoint,
                Eigen::Dynamic,
                Eigen::Dynamic,
                Eigen::RowMajor>
            DataMatrix;

        void setData(
            const DataPoint *source_data,
            int width, int height
        )
        { 
            data = source_data;
            this->dw = width, this->dh = height;
        }

        void setUpperLeftCorner( int xl, int yl ) {
            this->xl = xl;
            this->yl = yl;
        }

      protected:
        const DataPoint* data;
        int dw, dh;
        int xl, yl;

        BaseLattice() { data = NULL; }

        inline DataPoint getPoint(int x, int y) 
            const 
            { return data[x + y*dw]; }

    };

    /** A LatticeFunction is a fit function that
     *  operates on a two-dimensional square lattice. The
     *  lattice distance is 1 in x- and y-direction,
     *  and the lattice size is given in the constructor
     *  as well as the data type.
     *
     *  The values are given using the setDataLine
     *  functions.
     **/
    template <typename DataPoint, int W, int H>
    class SelectableLatticeFunction 
    : public BaseLattice<DataPoint> 
    {
      public:
        Eigen::Matrix<double,H,W> selectedData;

        void setUpperLeftCorner(int xl, int yl) {
            BaseLattice<DataPoint>::setUpperLeftCorner
                (xl,yl);
            selectedData = 
                BaseLattice<DataPoint>::DataMatrix
                ::Map(this->data, this->dh, this->dw)
                .template block<H,W>( yl, xl )
                .template cast<double>();
        }

        int getWidth() const { return W; }
        int getHeight() const { return H; }

        inline void setSize( int xs, int ys ) {
            assert( xs == W && ys == H );
        }
    };

    template <typename DataPoint>
    class SelectableLatticeFunction
        <DataPoint,Eigen::Dynamic,Eigen::Dynamic>
    : public BaseLattice<DataPoint>
    {
      public:
        Eigen::MatrixXd selectedData;
        void setUpperLeftCorner(int xl, int yl) 
        {
            BaseLattice<DataPoint>::setUpperLeftCorner
                (xl,yl);
            selectedData = 
                BaseLattice<DataPoint>::DataMatrix
                ::Map(this->data, this->dh, this->dw)
                .template block( yl, xl, Height, Width )
                .template cast<double>();
        }

        void setSize( int xs, int ys ) {
            this->Width = xs; this->Height = ys;
            selectedData.resize( this->Height, this->Width );
        }

        int getWidth() const { return Width; }
        int getHeight() const { return Height; }

      protected:
        int Width, Height;
    };

    template <typename DataPoint, int Width, int Height>
    struct LatticeFunction
    : public SelectableLatticeFunction
        <DataPoint,Width,Height>
    {
        template <typename Position, typename Derivatives,
                  typename Constants, typename Deriver>
        bool compute_derivatives(
            Position& position, 
            Derivatives& derivatives,
            const Constants& constants,
            Deriver& my_deriver) const
        {
            bool position_ok
                = my_deriver.prepare
                    ( position.parameters, constants,
                                   this->xl, this->yl );
            if ( !position_ok ) return false;
            my_deriver.compute( 
                this->selectedData,
                position.residues,
                derivatives.beta,
                derivatives.alpha
            );
            position.chi_sq 
                = position.residues.cwise().square().sum();
            return true;
        }
        
        DataPoint getCorner(int xfac, int yfac) {
            const int x = this->xl+
                ((xfac == 1) ? this->getWidth()-1 : 0);
            const int y = this->yl+
                ((yfac == 1) ? this->getHeight()-1 : 0);
            return this->getPoint(x, y);
        }
    };
}

#endif

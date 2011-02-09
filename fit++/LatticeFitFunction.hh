#ifndef LIBFITPP_LATTICEFITFUNCTION_H
#define LIBFITPP_LATTICEFITFUNCTION_H

#include <Eigen/StdVector>
#include "debug.h"
#include <fit++/FitFunction.hh>
#include <cassert>
#include <algorithm>

namespace fitpp {
    template <int VarCount, int Width, int Height, int Depth>
    struct LatticePosition
    : public Position<VarCount>
    {
        typedef std::vector< Eigen::Matrix<double,Height,Width> >
            ResidueType;
        ResidueType residues;

        LatticePosition() : residues( Depth ) {}

        void resize(int width, int height) { 
            DEBUG( "Resizing vector of size " << residues.size() << " to " << width << " " << height );
            for (int i = 0; i < Depth; ++i)
                residues[i].resize(height,width); 
            DEBUG("Resized");
        }
        void resize( const LatticePosition& o ) {
            DEBUG( "Resizing vector of size " << residues.size() << " from vector of size " << o.residues.size());
            for (int i = 0; i < Depth; ++i)
                residues[i].resize( o.residues[i].rows(), o.residues[i].cols()); 
            DEBUG("Resized");
        }

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    template <typename DataPoint, int _Depth>
    class BaseLattice {
      public:
        static const int Depth = _Depth;
        typedef 
            Eigen::Matrix<DataPoint,
                Eigen::Dynamic,
                Eigen::Dynamic,
                Eigen::RowMajor>
            DataMatrix;

        void setData(
            const DataPoint *source_data,
            int width, int height, int z_offset
        )
        { 
            data = source_data;
            dw = width, dh = height, this->z_offset = z_offset;
        }

        void setUpperLeftCorner( int xl, int yl ) {
            this->xl = xl;
            this->yl = yl;
        }

      protected:
        const DataPoint* data;
        int dw, dh, z_offset;
        int xl, yl;

        BaseLattice() { data = NULL; }

        inline DataPoint getPoint(int x, int y, int z) 
            const 
            { return data[x + y*dw + z_offset*z ]; }

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
    template <typename BaseLattice, int W, int H>
    class SelectableLatticeFunction 
    : public BaseLattice 
    {
      public:
        std::vector< Eigen::Matrix<double,H,W> >
            selectedData;

        SelectableLatticeFunction() : selectedData(BaseLattice::Depth) {}

        void setUpperLeftCorner(int xl, int yl) {
            BaseLattice::setUpperLeftCorner
                (xl,yl);
            for (int i = 0; i < BaseLattice::Depth; ++i)
                selectedData[i] = 
                    BaseLattice::DataMatrix::Map(this->data+i*this->z_offset, this->dh, this->dw)
                    .template block<H,W>( yl, xl )
                    .template cast<double>();
        }

        int getWidth() const { return W; }
        int getHeight() const { return H; }

        inline void setSize( int xs, int ys ) {
            assert( xs == W && ys == H );
        }
    };

    template <typename BaseLattice>
    class SelectableLatticeFunction
        <BaseLattice,Eigen::Dynamic,Eigen::Dynamic>
    : public BaseLattice
    {
      public:
        std::vector< Eigen::MatrixXd > selectedData;
        SelectableLatticeFunction() : selectedData(BaseLattice::Depth) {}
        void setUpperLeftCorner(int xl, int yl) 
        {
            BaseLattice::setUpperLeftCorner
                (xl,yl);
            for (int i = 0; i < BaseLattice::Depth; ++i)
                selectedData[i] = 
                    BaseLattice::DataMatrix
                    ::Map(this->data+i*this->z_offset, this->dh, this->dw)
                    .template block( yl, xl, Height, Width )
                    .template cast<double>();
        }

        void setSize( int xs, int ys ) {
            this->Width = xs; this->Height = ys;
            for (int i = 0; i < BaseLattice::Depth; ++i)
                selectedData[i].resize( this->Height, this->Width );
        }

        int getWidth() const { return Width; }
        int getHeight() const { return Height; }

      protected:
        int Width, Height;
    };

    template <typename DataPoint, int Width, int Height, int Depth>
    struct LatticeFunction
    : public SelectableLatticeFunction
        < BaseLattice<DataPoint,Depth>,Width,Height>
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
                    ( position.parameters, constants, this->xl, this->yl, 0 );
            if ( !position_ok ) return false;
            my_deriver.compute( 
                this->selectedData[0],
                position.residues[0],
                derivatives.beta,
                derivatives.alpha
            );
            position.chi_sq 
                = position.residues[0].cwise().square().sum();
            for (int i = 1; i < Depth; ++i) {
                Derivatives v;

                position_ok = my_deriver.prepare
                        ( position.parameters, constants, this->xl, this->yl, i );
                if ( ! position_ok ) return false;
                my_deriver.compute( 
                    this->selectedData[i],
                    position.residues[i],
                    v.beta, v.alpha );

                derivatives.alpha += v.alpha;
                derivatives.beta += v.beta;
                position.chi_sq += position.residues[i].cwise().square().sum();
            }
            return true;
        }
        
        DataPoint getCorner(int xfac, int yfac, int depth) {
            const int x = this->xl+
                ((xfac == 1) ? this->getWidth()-1 : 0);
            const int y = this->yl+
                ((yfac == 1) ? this->getHeight()-1 : 0);
            return this->getPoint(x, y, depth);
        }
    };
}

#endif

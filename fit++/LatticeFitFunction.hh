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

        template <typename Type>
        void setData( const Type& data )
        { 
            this->data = data.ptr();
            for (int i = 0; i < Type::Dim; ++i) {
                size[i] = data.sizes()[i].value();
                delta[i] = data.get_offsets()[i];
            }
            for (int i = Type::Dim; i < 3; ++i) {
                size[i] = 1;
                pos[i] = 0;
                delta[i] = 0;
            }
        }

        void setUpperLeftCorner( int xl, int yl ) {
            pos[0] = xl;
            pos[1] = yl;
            pos[2] = 0;
        }

      protected:
        const DataPoint* data;
        Eigen::Vector3i size, delta, pos;

        BaseLattice() { data = NULL; }

        inline DataPoint getPoint(int x, int y, int z) 
            const 
            { Eigen::Vector3i p; p << x,y,z; return data[ (p + pos).dot(delta) ]; }

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
                for (int x = 0; x < selectedData[i].cols(); ++x)
                    for (int y = 0; y < selectedData[i].rows(); ++y) {
                        selectedData[i](y,x) = this->getPoint(x,y,i);
                    }
        }

        int getWidth() const { return selectedData[0].cols(); }
        int getHeight() const { return selectedData[0].rows(); }

        void setSize( int xs, int ys ) {
            for (int i = 0; i < BaseLattice::Depth; ++i)
                selectedData[i].resize( ys, xs );
        }
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
                    ( position.parameters, constants, this->pos[0], this->pos[1], 0 );
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
                        ( position.parameters, constants, this->pos[0], this->pos[1], i );
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
            const int x = this->pos[0]+
                ((xfac == 1) ? this->getWidth()-1 : 0);
            const int y = this->pos[1]+
                ((yfac == 1) ? this->getHeight()-1 : 0);
            return this->getPoint(x, y, depth);
        }
    };
}

#endif
